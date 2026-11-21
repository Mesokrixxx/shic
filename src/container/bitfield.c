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

bool bitfield_eq(const Bitfield bf1, const Bitfield bf2) {
	const Bitfield first = bitfield_size(bf1) > bitfield_size(bf2) ? bf2 : bf1;
	const Bitfield second = first == bf1 ? bf2 : bf1;
	const u64 firstSize = bitfield_size(first);

	dynlist_forEach(first, it) {
		if (*it.elem != second[it.idx])
			return false;
	}

	dynlist_forEach(second, firstSize, it) {
		if (*it.elem)
			return false;
	}

	return true;
}

bool bitfield_has(const Bitfield big, const Bitfield small) {
	const u64 bigSize = bitfield_size(big);
	const u64 smallSize = bitfield_size(small);

	if (bigSize > smallSize) {
		dynlist_forEach(small, it) {
			if ((big[it.idx] & *it.elem) != *it.elem)
				return false;
		}
	}

	else {
		dynlist_forEach(big, it) {
			if ((*it.elem & small[it.idx]) != small[it.idx])
				return false;
		}

		dynlist_forEach(small, bigSize, it) {
			if (*it.elem)
				return false;
		}
	}

	return true;
}

void bitfield_copy(Bitfield *destptr, const Bitfield src) {
	const u64 srcSize = bitfield_size(src);

	bitfield_reserve(destptr, srcSize);
	dynlist_forEach(src, it)
		(*destptr)[it.idx] = *it.elem;
	dynlist_forEach(*destptr, srcSize, it)
		*it.elem = 0;
}

void bitfield_reserve(Bitfield *bfptr, u64 reserve) {
	const u64 oCap = bitfield_capacity(*bfptr);

	if (!dynlist_reserve(bfptr, reserve))
		return ;

	dynlist_sizeSet(*bfptr, bitfield_capacity(*bfptr));
	dynlist_forEach(*bfptr, oCap, it)
		*it.elem = 0;
}
