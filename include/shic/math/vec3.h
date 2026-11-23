#pragma once

#include "shic/math/types.h"
#include "shic/core/macros.h"

#define v3(...) VMACRO(_v3, __VA_ARGS__)

// ---

#define _v30() _v33(0, 0, 0)
#define _v31(xyz) ({ typeof(xyz) _xyz = (xyz); _v33(_xyz); })
#define _v33(x, y, z) (V3){{ (x), (y), (z) }}
