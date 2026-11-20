#pragma once

#include "shic/container/dynlist.h"

typedef Dynlist(u64) Bitfield;

#define bitfield_setOn(bfptr, ...) _bitfield_setImpl((bfptr), true, (const u64[]){ __VA_ARGS__ }, NARG(__VA_ARGS__))
#define bitfield_setOff(bfptr, ...) _bitfield_setImpl((bfptr), false, (const u64[]){ __VA_ARGS__ }, NARG(__VA_ARGS__))

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

void _bitfield_setImpl(Bitfield *bfptr, bool status, const u64 *idxs, u64 idxCount);
