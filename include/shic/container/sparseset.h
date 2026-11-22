#pragma once

#include "shic/container/dynlist.h"

typedef struct {
	Allocator *allocator;
	Dynlist(u64) sparse;
	u64 *dense;
	u64 denseSize, denseCap, itemSize;
} SparseSet;

void sparseset_insert(SparseSet *ss, u64 id, const void *item);
void sparseset_delete(SparseSet *ss, u64 id);
void *sparseset_get(SparseSet *ss, u64 id);
bool sparseset_has(const SparseSet *ss, u64 id);
void sparseset_destroy(SparseSet *ss);

#define sparseset_create(...) VMACRO(_sparseset_create, __VA_ARGS__)

#define sparseset_getIdx(ss, id) ({ (ss)->sparse[(id)]; })
#define sparseset_getId(ss, idx) ({ (ss)->dense[(idx)]; })

// ---

SparseSet *_sparseset_createImpl(Allocator *allocator, u64 itemSize, u64 reserve);

#define _sparseset_create2(allocator, itemSize) _sparseset_createImpl((allocator), (itemSize), 0)
#define _sparseset_create3(allocator, itemSize, reserve) _sparseset_createImpl((allocator), (itemSize), (reserve))
