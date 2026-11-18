#include "shic/container/dynlist.h"
#include "shic/math/util.h"
#include "shic/types/defs.h"
#include <string.h>

#define DYNLIST_MIN_SIZE 8

static u64 headerSize() {
	return math_roundupPow2(sizeof(DynlistHeader), MAX_ALIGN);
}

static DynlistHeader *headerGet(void *list) {
	return (DynlistHeader *)((u8 *)list - headerSize());
}

static const DynlistHeader *headerCGet(const void *list) {
	return headerGet((void *)list);
}

void dynlist_pushBackArray(void *listptr, const void *arr, u64 arrSize) {
	dynlist_reserve(listptr, dynlist_size(*(void **)listptr) + arrSize);

	DynlistHeader *header = headerGet(*(void **)listptr);

	memcpy(
		(u8 *)header + headerSize() + header->used * header->typeSize, 
		arr, arrSize * header->typeSize);
	header->used += arrSize;
}

bool dynlist_reserve(void *listptr, u64 reserve) {
	void **list = (void **)listptr;
	DynlistHeader *header = headerGet(*list);

	u64 nCap = header->reserved;
	if (nCap >= reserve)
		return false;

	while (nCap < reserve)
		nCap *= 2;

	DynlistHeader *nHeader = 
		allocator_alloc(header->allocator, headerSize() + nCap * header->typeSize);
	*nHeader = (DynlistHeader){
		.allocator = header->allocator,
		.used = header->used,
		.reserved = nCap,
		.typeSize = header->typeSize
	};

	void *nList = (u8 *)nHeader + headerSize();

	memcpy(nList, *list, nHeader->used * nHeader->typeSize);
	allocator_free(nHeader->allocator, header);
	*list = nList;
	return true;
}

void dynlist_sizeSet(void *list, u64 size) {
	headerGet(list)->used = size;
}

u64 dynlist_size(const void *list) {
	return headerCGet(list)->used; 
}

u64 dynlist_capacity(const void *list) {
	return headerCGet(list)->reserved;
}

Allocator *dynlist_allocator(void *list) {
	return headerGet(list)->allocator;
}

void dynlist_destroy(void *list) {
	if (!list)
		return ;
	
	DynlistHeader *header = headerGet(list);

	allocator_free(header->allocator, header);
}

void _dynlist_initImpl(Allocator *allocator, void *listptr, u64 reserve, u64 typeSize) {
	void **list = (void **)listptr;

	reserve = 
		math_max(math_roundupPow2(reserve, 2), DYNLIST_MIN_SIZE);
	DynlistHeader *header = allocator_alloc(allocator, headerSize() + reserve * typeSize);
	*header = (DynlistHeader){
		.allocator = allocator,
		.reserved = reserve,
		.typeSize = typeSize
	};

	*list = (u8 *)header + headerSize();
}

void _dynlist_pushBackImpl(void *listptr, const void *x, u64 n) {
	dynlist_reserve(listptr, dynlist_size(*(void **)listptr) + n);

	DynlistHeader *header = headerGet(*(void **)listptr);

	for (u64 i = 0; i < n; i++) {
		memcpy(
			(u8*)header + headerSize() + (header->used + i) * header->typeSize,
			x, header->typeSize);
	}
	header->used += n;
}
