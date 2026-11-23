#pragma once

#include "shic/math/types.h"
#include "shic/core/macros.h"

#define v4(...) VMACRO(_v4, __VA_ARGS__)

// ---

#define _v40() _v44(0, 0, 0, 0)
#define _v41(xyzw) ({ typeof(xyzw) _xyzw = (xyzw); _v44(_xyzw, _xyzw, _xyzw, _xyzw); })
#define _v44(x, y, z, w) (V4){{ (x), (y), (z), (w) }}
