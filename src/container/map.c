#include "shic/container/map.h"
#include "shic/math/util.h"
#include "shic/types/defs.h"
#include "shic/debug/assert.h"
#include <string.h>

typedef struct {
	Hash hash;
	u16 dist;
	bool used;
} MapEntry;

typedef enum : u8 {
	REHASH_GROW = 1 << 0,
	REHASH_SHRINK = 1 << 1,
	REHASH_FORCE = 1 << 2
} RehashMethod;

#define LOAD_HIGH 0.85
#define LOAD_LOW 0.15

#define DIST_MAX UINT16_MAX

static const u32 PRIMES[] = {
    11, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189, 805306457, 1610612741
};

#define PRIMES_COUNT ARRAY_SIZE(PRIMES)
#define INEXISTANT_KEY (PRIMES[PRIMES_COUNT - 1] + 1)

static MapEntry *entries(Map *map) {
	return map->data;
}

static void *keys(Map *map) {
	return 
		(u8 *)entries(map) 
		+ math_roundupPow2(map->cap * sizeof(MapEntry), MAX_ALIGN);
}

static void *keyAt(Map *map, u64 idx) {
	return 
		(u8 *)keys(map)
		+ map->keySize * idx;
}

static void *values(Map *map) {
	return 
		(u8 *)keys(map)
		+ math_roundupPow2(map->cap * map->keySize, MAX_ALIGN);
}

static void *valueAt(Map *map, u64 idx) {
	return 
		(u8 *)values(map)
		+ map->valueSize * idx;
}

static void *spares(Map *map) {
	return 
		(u8 *)values(map)
		+ math_roundupPow2(map->cap * map->valueSize, MAX_ALIGN);
}

static void *spareKey(Map *map) {
	return 
		(u8 *)spares(map)
		+ map->keySize * map->spare;
}

static void *spareValue(Map *map) {
	return
		(u8 *)spares(map)
		+ math_roundupPow2(2 * map->keySize, MAX_ALIGN)
		+ map->valueSize * map->spare;
}

static void *dataCreate(Map *map) {
	const u64 entrySize = map->cap * sizeof(MapEntry);
	const u64 totSize = 
		math_roundupPow2(entrySize, MAX_ALIGN)
		+ math_roundupPow2(map->cap * map->keySize, MAX_ALIGN)
		+ math_roundupPow2(map->cap * map->valueSize, MAX_ALIGN)
		+ math_roundupPow2(2 * map->keySize, MAX_ALIGN)
		+ math_roundupPow2(2 * map->valueSize, MAX_ALIGN);
	
	void *data = allocator_alloc(map->allocator, totSize);
	
	memset(data, 0, entrySize);
	return data;
}

static void insertWithHash(Map *map, const void *key, const void *value, Hash hash);

static void dataRehash(Map *map, RehashMethod method) {
	if (map->rehashLocked)
		return ;

	const u64 oCap = map->cap;
	const f32 load = oCap ? map->size / (f32)oCap : 0;

	const bool force = method & REHASH_FORCE;
	const bool grow = method & REHASH_GROW && (force || load > LOAD_HIGH);
	const bool shrink = method & REHASH_SHRINK && (force || load < LOAD_LOW);

	if ((!grow && !shrink)
			|| (!force && ((grow && map->prime == PRIMES_COUNT - 1) || (shrink && !map->prime))))
		return ;

	assert(!(force && ((grow && map->prime == PRIMES_COUNT - 1) || (shrink && !map->prime))),
		"attempted rehash but can't {}", grow ? "grow" : "shrink");

	if (shrink)
		map->prime--;
	else
	 	map->prime++;

	void *oData = map->data;
	MapEntry *oEntries = entries(map);
	void *oKeys = keys(map); 
	void *oValues = values(map);

	map->cap = PRIMES[map->prime];
	map->size = 0;
	map->data = dataCreate(map);

	if (!oData)
		return ;

	map->rehashLocked = true;
	for (u64 i = 0; i < oCap; i++) {
		if (!oEntries[i].used)
			continue ;
		
		void *key = (u8 *)oKeys + i * map->keySize;
		void *value = (u8 *)oValues + i * map->valueSize;

		insertWithHash(map, key, value, oEntries[i].hash);
	}
	map->rehashLocked = false;

	allocator_free(map->allocator, oData);
}

static void insertWithHash(Map *map, const void *key, const void *value, Hash hash) {
	u64 pos = hash % map->cap;
	u16 dist = 0;

	bool isNew = true;

	while (true) {
		MapEntry *entry = &entries(map)[pos];
	
		if (!entry->used) {
			*entry = (MapEntry){
				.dist = dist,
				.hash = hash,
				.used = true,
			};

			memcpy(keyAt(map, pos), key, map->keySize);
			memcpy(valueAt(map, pos), value, map->valueSize);
			break ;
		}

		else if (entry->dist == dist && entry->hash == hash && map->cmpF(map, key, keyAt(map, pos))) {
			isNew = false;

			memcpy(keyAt(map, pos), key, map->keySize);
			memcpy(valueAt(map, pos), value, map->valueSize);
			break ;
		}

		else if (entry->dist < dist) {
			if (isNew) {
				map->size++;
				isNew = false;
			}

			MapEntry oEntry = *entry;
			*entry = (MapEntry){
				.dist = dist,
				.hash = hash,
				.used = true
			};

			void *sKey = spareKey(map), *mKey = keyAt(map, pos);
			void *sValue = spareValue(map), *mValue = valueAt(map, pos);

			memcpy(sKey, mKey, map->keySize);
			memcpy(sValue, mValue, map->valueSize);
			memcpy(mKey, key, map->keySize);
			memcpy(mValue, value, map->valueSize);

			dist = oEntry.dist;
			hash = oEntry.hash;
			key = sKey;
			value = sValue;

			map->spare = 1 - map->spare;
		}

		dist++;
		if (dist >= DIST_MAX) {
			assert(!map->rehashLocked,
				"edge cases reached, need fixing");

			u8 *tmp = allocator_alloc(map->allocator, map->keySize + map->valueSize);
			void *tmpKey = tmp;
			void *tmpValue = tmp + map->keySize;

			memcpy(tmpKey, key, map->keySize);
			memcpy(tmpValue, value, map->valueSize);

			dataRehash(map, REHASH_FORCE | REHASH_GROW);
			map_insert(map, tmpKey, tmpValue);

			allocator_free(map->allocator, tmp);
			return ;
		}

		pos = (pos + 1) % map->cap;
	}

	if (isNew)
		map->size++;

	dataRehash(map, REHASH_GROW);
}

static u64 findKey(Map *map, const void *key) {
	if (!map->data)
		return INEXISTANT_KEY;

	const Hash hash = map->hashF(map, key);
	
	u64 pos = hash % map->cap;
	u16 dist = 0;

	while (true) {
		MapEntry entry = entries(map)[pos];

		if (!entry.used || entry.dist < dist)
			return INEXISTANT_KEY;

		if (entry.hash == hash && map->cmpF(map, key, keyAt(map, pos)))
			return pos;

		dist++;
		pos = (pos + 1) % map->cap;
	}
}

Map *map_create(Allocator *allocator, u64 keySize, u64 valueSize, MapHashFn hashF, MapCmpFn cmpF) {
	Map *map = allocator_alloc(allocator, sizeof(Map));
	*map = (Map){
		.allocator = allocator,
		.hashF = hashF,
		.cmpF = cmpF,
		.keySize = keySize,
		.valueSize = valueSize
	};

	return map;
}

void map_insert(Map *map, const void *key, const void *value) {
	if (!map->data) {
		map->cap = PRIMES[map->prime];
		map->data = dataCreate(map);
	}

	insertWithHash(map, key, value, map->hashF(map, key));
}

void *map_get(Map *map, const void *key) {
	const u64 pos = findKey(map, key);

	return pos != INEXISTANT_KEY ? valueAt(map, pos) : nullptr;
}

void map_delete(Map *map, const void *key) {
	u64 pos = findKey(map, key);
	
	if (pos == INEXISTANT_KEY)
		return ;

	MapEntry *mapEntries = entries(map);

	mapEntries[pos].used = false;

	while (true) {
		u64 nextPos = (pos + 1) % map->cap;
		
		if (!mapEntries[nextPos].used || !mapEntries[nextPos].dist)
			break ;

		mapEntries[pos] = mapEntries[nextPos];
		mapEntries[pos].dist--;

		memcpy(keyAt(map, pos), keyAt(map, nextPos), map->keySize);
		memcpy(valueAt(map, pos), valueAt(map, nextPos), map->valueSize);

		mapEntries[nextPos].used = false;
		pos = nextPos;
	}

	map->size--;
	dataRehash(map, REHASH_SHRINK);
}

void map_destroy(Map *map) {
	if (!map)
		return ;
	allocator_free(map->allocator, map->data);
	allocator_free(map->allocator, map);
}

Hash map_fixstrHash(Map *map, const void *str) {
	return hash_add_bytes(0xdeadbeef, str, map->keySize);
}

bool map_fixstrCmp(Map *map, const void *s1, const void *s2) {
	return !memcmp(s1, s2, map->keySize);	
}
