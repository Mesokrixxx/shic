#pragma once

#include "shic/memory/allocator.h"
#include "shic/core/macros.h"

typedef struct {
	Allocator *allocator;
	u64 used, reserved;
	u64 typeSize;
} DynlistHeader;

#define Dynlist(t) t*

void dynlist_pushBackArray(void *listptr, const void *arr, u64 arrSize);
bool dynlist_reserve(void *listptr, u64 reserve);
void dynlist_sizeSet(void *list, u64 size);
u64 dynlist_size(const void *list);
u64 dynlist_capacity(const void *list);
Allocator *dynlist_allocator(void *list);
void dynlist_destroy(void *list);

#define dynlist_clear(list) dynlist_sizeSet((list), 0)
#define dynlist_forEach(list, it) _dynlist_forEachImpl((list), it, CONCAT(_list, __COUNTER__), CONCAT(_it, __COUNTER__))

// @use    dynlist_init(allocator, listptr[, reserve])
// @return -
// @desc   init list with 'reserve' slots, DYNLIST_MIN_SIZE by default
#define dynlist_init(...) VMACRO(_dynlist_init, __VA_ARGS__)

// @use    dynlist_pushback(listptr, x[, n])
// @return -
// @desc   x will be pushed n times, 1 by default
#define dynlist_pushBack(...) VMACRO(_dynlist_pushBack, __VA_ARGS__)

// ---

void _dynlist_initImpl(Allocator *allocator, void *listptr, u64 reserve, u64 typeSize);
void _dynlist_pushBackImpl(void *listptr, const void *x, u64 n);

#define _dynlist_forEachImpl(list, it, listname, itname) \
	typedef struct { \
		u64 idx, end; \
		typeof_unqual(list) elem; \
	} itname; \
	typeof(list) listname = (list); \
	for (itname it = { \
		.end = listname ? dynlist_size(listname) : 0, \
		.elem = listname, \
	}; it.idx < it.end; \
	it.elem = listname + ++it.idx)

#define _dynlist_init2(allocator, listptr) _dynlist_initImpl((allocator), (listptr), 0, sizeof(**(listptr)))
#define _dynlist_init3(allocator, listptr, reserve) _dynlist_initImpl((allocator), (listptr), (reserve), sizeof(**(listptr)))
#define _dynlist_pushBack2(listptr, x) _dynlist_pushBackImpl((listptr), (x), 1)
#define _dynlist_pushBack3(listptr, x, n) _dynlist_pushBackImpl((listptr), (x), (n))
