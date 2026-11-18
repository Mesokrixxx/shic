#include "shic/memory/allocator.h"
#include "shic/debug/assert.h"
#include "shic/error/error.h"

void *allocator_alloc(Allocator *alloc, u64 size) {
	void *ptr = alloc->_allocImpl(alloc, size);
	assert(ptr, "failed to alloc {} zu bytes: {}", 
		size, error_get());

	return ptr;
}

void allocator_free(Allocator *alloc, void *ptr) {
	if (!ptr)
		return ;
	alloc->_freeImpl(alloc, ptr);
}
