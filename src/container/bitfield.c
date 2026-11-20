#include "shic/container/bitfield.h"

static void bitSet(Bitfield *bfptr, bool status, u64 idx) {
	const u64 row = idx / 64;
	const u64 off = idx % 64;
	const u64 mask = (u64)1 << off; 

	u64 oBfCap = dynlist_capacity(*bfptr);
	if (dynlist_reserve(bfptr, row)) {
		dynlist_sizeSet(*bfptr, dynlist_capacity(*bfptr));
		dynlist_forEach(*bfptr, oBfCap, it)
			*it.elem = 0;
	}

	if (status)
		(*bfptr)[row] |= mask;
	else
	 	(*bfptr)[row] &= ~mask;
}

void _bitfield_setImpl(Bitfield *bfptr, bool status, const u64 *idxs, u64 idxCount) {
	for (u64 i = 0; i < idxCount; i++)
		bitSet(bfptr, status, idxs[i]);
}

bool bitfield_status(const Bitfield bf, u64 idx) {
	const u64 row = idx / 64;
	const u64 off = idx % 64;
	const u64 mask = (u64)1 << off;

	return row < dynlist_capacity(bf) ? !!(bf[row] & mask) : false;
}
