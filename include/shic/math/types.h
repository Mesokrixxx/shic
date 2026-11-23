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

typedef union {
	struct { f32 x, y, z; };
	struct { V2 xy; f32 _z; };
	struct { V2 _x; f32 yz; };
	f32 raw[3];
} V3;

typedef union {
	struct { f32 x, y, z, w; };
	struct { f32 r, g, b, a; };
	struct { V2 xy, zw; };
	struct { V2 rg, ba; };
	struct { f32 _x; V2 yz; f32 _w; };
	struct { f32 _r; V2 gb; f32 _a; };
	struct { V3 xyz; f32 __w; };
	struct { V3 rgb; f32 __a; };
	struct { f32 __x; V3 yzw; };
	struct { f32 __r; V3 gba; };
	f32 raw[4];
} V4;
