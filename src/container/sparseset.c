#include "shic/container/sparseset.h"
#include "shic/math/util.h"
#include "shic/types/defs.h"
#include "shic/debug/assert.h"
#include <string.h>

#define SPARSESET_UNUSED_ID UINT64_MAX

static void *denseCreate(Allocator *allocator, u64 cap, u64 itemSize) {
	return 
		allocator_alloc(allocator,
			math_roundupPow2(sizeof(u64) * cap, MAX_ALIGN)
			+ math_roundupPow2(itemSize * cap, MAX_ALIGN));
}

static void *getItem(void *dense, u64 cap, u64 idx, u64 itemSize) {
	return 
		(u8 *)dense 
		+ math_roundupPow2(sizeof(u64) * cap, MAX_ALIGN)
		+ idx * itemSize;
}

SparseSet *_sparseset_createImpl(Allocator *allocator, u64 itemSize, u64 reserve) {
	Dynlist(u64) sparse;
	dynlist_init(allocator, &sparse, math_max(DYNLIST_MIN_SIZE, reserve));
	dynlist_sizeSet(sparse, dynlist_capacity(sparse));
	dynlist_forEach(sparse, it)
		*it.elem = SPARSESET_UNUSED_ID;

	void *dense = denseCreate(allocator, DYNLIST_MIN_SIZE, itemSize);

	SparseSet *ss = allocator_alloc(allocator, sizeof(SparseSet));
	*ss = (SparseSet){
		.allocator = allocator,
		.sparse = sparse,
		.dense = dense,
		.denseCap = DYNLIST_MIN_SIZE,
		.itemSize = itemSize,
	};

	return ss;
}

void sparseset_insert(SparseSet *ss, u64 id, const void *item) {
	assert(id != SPARSESET_UNUSED_ID,
		"id can't be equal to '{}' because it is used to reference an unused slot", 
		SPARSESET_UNUSED_ID);

	u64 oldSparseCap = dynlist_capacity(ss->sparse);
	if (dynlist_reserve(&ss->sparse, id + 1)) {
		dynlist_sizeSet(ss->sparse, dynlist_capacity(ss->sparse));
		dynlist_forEach(ss->sparse, oldSparseCap, it)
			*it.elem = SPARSESET_UNUSED_ID;
	}

	u64 idx = ss->sparse[id];
	if (idx == SPARSESET_UNUSED_ID) {
		idx = ss->denseSize++;
		ss->sparse[id] = idx;
	}

	if (ss->denseSize > ss->denseCap) {
		u64 newDenseCap = ss->denseCap * 2;
		void *newDense = denseCreate(ss->allocator, newDenseCap, ss->itemSize);

		memcpy(newDense, ss->dense, sizeof(u64) * ss->denseSize);
		memcpy(
			getItem(newDense, newDenseCap, 0, ss->itemSize), 
			getItem(ss->dense, ss->denseCap, 0, ss->itemSize),
			ss->itemSize * ss->denseSize);

		allocator_free(ss->allocator, ss->dense);

		ss->denseCap = newDenseCap;
		ss->dense = newDense;
	}
	
	memcpy(
		getItem(ss->dense, ss->denseCap, idx, ss->itemSize),
		item, ss->itemSize);
	ss->dense[idx] = id;
}

void sparseset_delete(SparseSet *ss, u64 id) {
	if (id >= dynlist_capacity(ss->sparse))
		return ;

	u64 idx = ss->sparse[id];
	if (idx == SPARSESET_UNUSED_ID)
		return ;

	u64 lastIdx = ss->denseSize - 1;
	u64 lastId = ss->dense[lastIdx];
	if (idx != lastIdx) {
		ss->dense[idx] = lastId;
		ss->sparse[lastId] = idx;
		
		memcpy(
			getItem(ss->dense, ss->denseCap, idx, ss->itemSize),
			getItem(ss->dense, ss->denseCap, lastIdx, ss->itemSize),
			ss->itemSize);
	}

	ss->sparse[id] = SPARSESET_UNUSED_ID;
	ss->denseSize--;
}

bool sparseset_has(const SparseSet *ss, u64 id) {
	return id < dynlist_capacity(ss->sparse) && ss->sparse[id] != SPARSESET_UNUSED_ID;
}

void *sparseset_get(SparseSet *ss, u64 id) {
	if (id >= dynlist_capacity(ss->sparse))
		return nullptr;

	u64 idx = ss->sparse[id];
	if (idx == SPARSESET_UNUSED_ID)
		return nullptr;

	return getItem(ss->dense, ss->denseCap, idx, ss->itemSize);
}

void sparseset_destroy(SparseSet *ss) {
	if (!ss)
		return ;
	dynlist_destroy(ss->sparse);
	allocator_free(ss->allocator, ss->dense);
	allocator_free(ss->allocator, ss);
}
