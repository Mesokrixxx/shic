#pragma once

#include "shic/types/alias.h"
#include "shic/core/macros.h"

typedef u64 Hash;

#define _HASH_ADD_LIST(X) \
	X(char) \
	X(bool) \
	X(u8) \
	X(u16) \
	X(u32) \
	X(u64) \
	X(i8) \
	X(i16) \
	X(i32) \
	X(i64)

#define X(t) Hash CONCAT(hash_add_, t)(Hash hash, t x); 
_HASH_ADD_LIST(X)
#undef X

Hash hash_add_str(Hash hash, const char *str);
Hash hash_add_bytes(Hash hash, const void *ptr, u64 n);
