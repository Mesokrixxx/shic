#pragma once

#include "shic/memory/allocator.h"
#include "shic/types/hash.h"

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

Map *map_create(Allocator *allocator, u64 keySize, u64 valueSize, MapHashFn hashF, MapCmpFn cmpF);
void map_insert(Map *map, const void *key, const void *value);
void *map_get(Map *map, const void *key);
void map_delete(Map *map, const void *key);
void map_destroy(Map *map);

Hash map_fixstrHash(Map *map, const void *str);
bool map_fixstrCmp(Map *map, const void *s1, const void *s2);
