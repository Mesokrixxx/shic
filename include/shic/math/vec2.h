#pragma once

#include "shic/core/macros.h"
#include "shic/types/alias.h"

typedef union {
	struct { f32 x, y; };
	f32 raw[2];
} V2;

typedef union {
	struct { i32 x, y; };
	i32 raw[2];
} V2i;

// @use    v2() | v2(xy) | v2(x, y)
// @return V2
// @desc   v2() is equivalent to v2(0), v2(xy) is equivalent to v2(xy, xy)
#define v2(...) VMACRO(_v2, __VA_ARGS__)

// @use    v2i() | v2i(xy) | v2i(x, y)
// @return V2i
// @desc   v2i() is equivalent to v2i(0), v2i(xy) is equivalent to v2i(xy, xy)
#define v2i(...) VMACRO(_v2i, __VA_ARGS__)

// ---

#define _v20() _v22(0, 0)
#define _v21(xy) ({ f32 _xy = (xy); _v22(_xy, _xy); })
#define _v22(x, y) (V2){{ (x), (y) }}

#define _v2i0() _v2i2(0, 0)
#define _v2i1(xy) ({ i32 _xy = (xy); _v2i2(_xy, _xy); })
#define _v2i2(x, y) (V2i){{ (x), (y) }}
