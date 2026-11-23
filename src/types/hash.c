#include "shic/types/hash.h"

#define X(t) Hash CONCAT(hash_add_, t)(Hash hash, t x) { \
		return (hash ^ ((Hash)(x) + 0x9E3779B9u + (hash << 6) + (hash >> 2))); \
	}
_HASH_ADD_LIST(X)
#undef X

Hash hash_add_str(Hash hash, const char *str) {
	while (*str)
		hash = hash_add_char(hash, *str++);
	return hash;
}

Hash hash_add_bytes(Hash hash, const void *ptr, u64 n) {
	for (u64 i = 0; i < n; i++)
		hash = hash_add_u8(hash, ((u8 *)ptr)[i]);
	return hash;
}
