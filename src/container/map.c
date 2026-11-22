#include "shic/container/map.h"
#include "shic/math/util.h"
#include "shic/types/defs.h"
#include "shic/debug/assert.h"
#include <string.h>

typedef struct {
	u16 dist : 16;
	u16 hash : 15;
	bool used : 1;
} MapEntry;

typedef enum : u8 {
	REHASH_GROW = 1 << 0,
	REHASH_SHRINK = 1 << 1,
	REHASH_FORCE = 1 << 2
} RehashMethod;

#define LOAD_HIGH 0.85
#define LOAD_LOW 0.15

#define DIST_MAX UINT16_MAX

#define STORED_HASH_MASK ((1 << 15) - 1)

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
		(u8 *)keys(map);
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

	assert(force && ((grow && map->prime == PRIMES_COUNT - 1) || (shrink && !map->prime)),
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

	u64 entrySize = map->cap * sizeof(MapEntry);
	u64 totSize = 
		math_roundupPow2(entrySize, MAX_ALIGN)
		+ math_roundupPow2(map->cap * map->keySize, MAX_ALIGN)
		+ math_roundupPow2(map->cap * map->valueSize, MAX_ALIGN)
		+ math_roundupPow2(2 * map->keySize, MAX_ALIGN)
		+ math_roundupPow2(2 * map->valueSize, MAX_ALIGN);
	map->data = allocator_alloc(map->allocator, totSize);
	memset(map->data, 0, entrySize);

	if (!oData)
		return ;

	map->rehashLocked = true;
	for (u64 i = 0; i < oCap; i++) {
		if (!oEntries[i].used)
			continue ;
		
		void *key = (u8 *)oKeys + i * map->keySize;
		void *value = (u8 *)oValues + i * map->valueSize;

		map_insert(map, key, value);
	}
	map->rehashLocked = false;
}

static u64 findKey(Map *map, const void *key) {
	if (!map->data)
		return INEXISTANT_KEY;

	const Hash hash = map->hashF(map, key);
	const Hash hashbits = hash & STORED_HASH_MASK;
	
	u64 pos = hash % map->cap;
	u16 dist = 0;

	while (true) {
		MapEntry entry = entries(map)[pos];

		if (!entry.used || dist >= DIST_MAX)
			return INEXISTANT_KEY;

		if (entry.hash == hashbits && map->cmpF(map, key, keyAt(map, pos)))
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
	if (!map->data)
		dataRehash(map, REHASH_FORCE | REHASH_GROW);

	Hash hash = map->hashF(map, key);
	Hash hashbits = hash & STORED_HASH_MASK;

	u64 pos = hash % map->cap;
	u16 dist = 0;

	bool isNew = true;

	while (true) {
		MapEntry *entry = &entries(map)[pos];
	
		if (!entry->used) {
			*entry = (MapEntry){
				.dist = dist,
				.hash = hashbits,
				.used = true,
			};

			memcpy(keyAt(map, pos), key, map->keySize);
			memcpy(valueAt(map, pos), value, map->valueSize);
			break ;
		}

		else if (entry->dist == dist && entry->hash == hashbits && map->cmpF(map, key, keyAt(map, pos))) {
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
				.hash = hashbits,
			};

			void *oKey = keyAt(map, pos);
			void *oValue = valueAt(map, pos);

			memcpy(spareKey(map), oKey, map->keySize);
			memcpy(spareValue(map), oValue, map->valueSize);

			memcpy(oKey, key, map->keySize);
			memcpy(oValue, value, map->valueSize);

			dist = oEntry.dist;
			hashbits = oEntry.hash;
			key = oKey;
			value = oValue;
			map->spare = 1 - map->spare;
		}

		dist++;
		if (dist >= DIST_MAX) {
			dataRehash(map, REHASH_FORCE | REHASH_GROW);
			map_insert(map, key, value);
			return ;
		}

		pos = (pos + 1) % map->cap;
	}

	if (isNew)
		map->size++;

	dataRehash(map, REHASH_GROW);
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

Hash map_fixstrHash(Map *, const void *str) {
	return hash_add_str(0xdeadbeef, str);
}

bool map_fixstrCmp(Map *map, const void *s1, const void *s2) {
	return strncmp(s1, s2, map->keySize);	
}
