#include "shic/memory/mallocator.h"
#include "shic/math/util.h"
#include "shic/error/error.h"
#include "shic/types/defs.h"
#include <stdlib.h>

typedef struct {
	u64 size;
} AllocHeader;

static u64 headerSize() {
	return math_roundupPow2(sizeof(AllocHeader), MAX_ALIGN);
}

static AllocHeader *headerGet(void *ptr) {
	return (AllocHeader *)((u8 *)ptr - headerSize());
}

static void *allocImpl(void *alloc, u64 size) {
	AllocHeader *header = malloc(headerSize() + size);
	if (!header) {
		error_setViaErrno("malloc");
		return NULL;
	}
	*header = (AllocHeader){ 
		.size = size
	};
	((Mallocator *)alloc)->reserved += header->size;
	return (u8 *)header + headerSize();
}

static void freeImpl(void *alloc, void *ptr) {
	AllocHeader *header = headerGet(ptr);

	((Mallocator *)alloc)->reserved -= header->size;
	free(header);
}

Mallocator g_mallocator = { ._base = { ._allocImpl = allocImpl, ._freeImpl = freeImpl } };
