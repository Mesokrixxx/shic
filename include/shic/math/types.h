#pragma once

#include "shic/types/alias.h"

typedef union {
	struct { f32 x, y; };
	f32 raw[2];
} V2;

typedef union {
	struct { i32 x, y; };
	i32 raw[2];
} V2i;
