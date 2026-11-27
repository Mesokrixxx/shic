#pragma once

#include "shic/memory/allocator.h"
#include "shic/types/hash.h"

typedef struct {
	Hash hash;
	u16 dist;
	bool used;
} MapEntry;

typedef struct map Map;

typedef Hash (*MapHashFn)(Map *, const void *);
typedef bool (*MapCmpFn)(Map *, const void *, const void *);

struct map {
	Allocator *allocator;
	MapHashFn hashF;
	MapCmpFn cmpF;

	u64 keySize, valueSize;
	u64 size, cap;
	
	void *data;
	
	u8 prime;
	bool rehashLocked : 1;
	u8 spare : 1;
};

#define MAP_INEXISTANT_KEY UINT64_MAX

Map *map_create(Allocator *allocator, u64 keySize, u64 valueSize, MapHashFn hashF, MapCmpFn cmpF);
void map_insert(Map *map, const void *key, const void *value);
void *map_get(Map *map, const void *key);
void map_delete(Map *map, const void *key);
void map_destroy(Map *map);

#define map_forEach(Tkey, Tval, map, it) _map_forEachImpl(Tkey, Tval, (map), it, CONCAT(_map, __COUNTER__), CONCAT(_it, __COUNTER__))

Hash map_fixstrHash(Map *map, const void *str);
bool map_fixstrCmp(Map *map, const void *s1, const void *s2);

// ---

void *_map_valueAt(Map *map, u64 idx);
void *_map_keyAt(Map *map, u64 idx);
u64 _map_findNextEntry(Map *map, u64 start);

#define _map_forEachBuildIt(mapname, itname, start) ({ \
		u64 _pos = _map_findNextEntry(mapname, (start)); \
		_pos != MAP_INEXISTANT_KEY ? \
			(itname){ \
				.key = _map_keyAt(mapname, _pos), \
				.elem = _map_valueAt(mapname, _pos), \
				.pos = _pos \
			} : (itname){ .pos = _pos }; \
	})

#define _map_forEachImpl(Tkey, Tval, map, it, mapname, itname) \
	typedef struct { \
		const Tkey key; \
		Tval *elem; \
		u64 pos; \
	} itname; \
	typeof(map) mapname = (map); \
	for (itname it = _map_forEachBuildIt(mapname, itname, 0); \
		it.pos != MAP_INEXISTANT_KEY; \
		it = _map_forEachBuildIt(mapname, itname, it.pos + 1))
