#include "shic/container/bitfield.h"

void bitfield_set(Bitfield *bfptr, u64 idx, bool active) {
	const u64 row = idx / 64;
	const u64 off = idx % 64;
	const u64 mask = (u64)1 << off; 

	u64 oBfCap = dynlist_capacity(*bfptr);
	if (dynlist_reserve(bfptr, row)) {
		dynlist_sizeSet(*bfptr, dynlist_capacity(*bfptr));
		dynlist_forEach(*bfptr, oBfCap, it)
			*it.elem = 0;
	}

	if (active)
		(*bfptr)[row] |= mask;
	else
	 	(*bfptr)[row] &= ~mask;
}

bool bitfield_status(const Bitfield bf, u64 idx) {
	const u64 row = idx / 64;
	const u64 off = idx % 64;
	const u64 mask = (u64)1 << off;

	return row < dynlist_capacity(bf) ? !!(bf[row] & mask) : false;
}
