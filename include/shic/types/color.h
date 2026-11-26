#pragma once

#include "shic/math/types.h"
#include "shic/core/macros.h"

typedef union {
	struct { u8 r, g, b, a; };
	u32 packed;
	u8 raw[4];
} Color;

#define color(...) VMACRO(_color, __VA_ARGS__) 

// ---

#define _color0() _color4(0, 0, 0, 1)
#define _color1(rgb) ({ typeof(rgb) _rgb = (rgb); _color4(_rgb, _rgb, _rgb, 1); })
#define _color3(r, g, b) _color4((r), (g), (b), 1)
#define _color4(r, g, b, a) (Color){{ (r), (g), (b), (a) }}
