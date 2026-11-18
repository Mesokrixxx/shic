#pragma once

#include "shic/types/alias.h"

#define math_roundupPow2(n, p) ({ typeof(n + p) _n = (n), _p = (p); (_n + (_p - 1)) & ~(_p - 1); })

#define math_max(a, b) ({ typeof(a + b) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define math_min(a, b) ({ typeof(a + b) _a = (a), _b = (b); _a < _b ? _a : _b; })

#define math_abs(x) ({ typeof(x) _x = (x); _x < 0 ? -_x : _x; })

f64 math_pow10(i64 exponent);
