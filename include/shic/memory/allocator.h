#pragma once

#include "shic/types/alias.h"

typedef struct {
	void *(*_allocImpl)(void *, u64);
	void (*_freeImpl)(void *, void *);
} Allocator;

#define allocator(alloc) ((Allocator *)&(alloc)->_base)

void *allocator_alloc(Allocator *alloc, u64 size);
void allocator_free(Allocator *alloc, void *ptr);
