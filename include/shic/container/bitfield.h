#pragma once

#include "shic/container/dynlist.h"

typedef Dynlist(u64) Bitfield;

void bitfield_set(Bitfield *bfptr, u64 idx, bool active);
bool bitfield_status(const Bitfield bf, u64 idx);

#define bitfield_init(allocator, bfptr, ...) \
	do { \
		typeof(bfptr) _bfptr = (bfptr); \
		dynlist_init((allocator), _bfptr, ##__VA_ARGS__); \
		dynlist_sizeSet(*_bfptr, dynlist_capacity(*_bfptr)); \
		dynlist_forEach(*_bfptr, it) \
			*it.elem = 0; \
	} while (0)

#define bitfield_capacity dynlist_capacity
#define bitfield_destroy dynlist_destroy

// ---

