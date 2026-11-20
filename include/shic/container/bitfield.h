#pragma once

#include "shic/container/dynlist.h"

typedef Dynlist(u64) Bitfield;

bool bitfield_status(const Bitfield bf, u64 idx);

#define bitfield_init(allocator, bfptr, ...) \
	do { \
		typeof(bfptr) _bfptr = (bfptr); \
		dynlist_init((allocator), _bfptr, ##__VA_ARGS__); \
		dynlist_sizeSet(*_bfptr, dynlist_capacity(*_bfptr)); \
		dynlist_forEach(*_bfptr, it) \
			*it.elem = 0; \
	} while (0)

#define bitfield_setOn(bfptr, ...) _bitfield_setImpl((bfptr), true, (const u64[]){ __VA_ARGS__ }, NARG(__VA_ARGS__))
#define bitfield_setOff(bfptr, ...) _bitfield_setImpl((bfptr), false, (const u64[]){ __VA_ARGS__ }, NARG(__VA_ARGS__))
#define bitfield_vsetOn(bfptr, idxs, idxCount) _bitfield_setImpl((bfptr), true, (idxs), (idxCount))
#define bitfield_vsetOff(bfptr, idxs, idxCount) _bitfield_setImpl((bfptr), false, (idxs), (idxCount))

#define bitfield_clear(bf) do { typeof(bf) _bf = (bf); dynlist_forEach(_bf, it) *it.elem = 0; } while (0)

#define bitfield_forEach(bf, it) _bitfield_forEachImpl((bf), it, CONCAT(_bf, __COUNTER__), CONCAT(_it, __COUNTER__))
#define bitfield_forEachSkipBlock(it) { (it).off = 63; continue ; }

#define bitfield_capacity dynlist_capacity
#define bitfield_destroy dynlist_destroy

// ---

void _bitfield_setImpl(Bitfield *bfptr, bool status, const u64 *idxs, u64 idxCount);

#define _bitfield_forEachImpl(bf, it, bfname, itname) \
	typedef struct { \
		bool bit; \
		u64 off, end; \
		u64 idx, value; \
	} itname; \
	typeof(bf) bfname = (bf); \
	for (itname it = { \
		.bit = *bfname & 1, \
		.end = bitfield_capacity(bfname) * 64, \
		.value = *bfname, \
	}; it.off < it.end; \
	it.off++, it.idx += it.off / 64, it.off %= 64, \
	it.value = bfname[it.idx], it.bit = (it.value >> it.off) & 1)
