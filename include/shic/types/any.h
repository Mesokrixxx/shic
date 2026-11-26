#pragma once

#include "shic/types/alias.h"
#include "shic/core/macros.h"
#include "shic/types/logic.h"
#include "shic/types/stringview.h"
#include "shic/math/types.h"
#include "shic/types/color.h"

#define _ANYTYPE_LIST(X) \
	X(UNKNOWN) \
	X(BOOL) \
	X(I8) \
	X(I16) \
	X(I32) \
	X(I64) \
	X(U8) \
	X(U16) \
	X(U32) \
	X(U64) \
	X(F32) \
	X(F64) \
	X(CHAR) \
	X(CSTR) \
	X(SV) \
	X(V2) \
	X(V2I) \
	X(V3) \
	X(V4) \
	X(COLOR) \
	X(PTR)

typedef enum {
#define X(name) CONCAT(ANYTYPE_, name),
	_ANYTYPE_LIST(X)
#undef X
	_ANYTYPE_COUNT
} AnyType;

typedef struct {
	AnyType type;
	union {
		bool boolean;
		i64 signedInt;
		u64 unsignedInt;
		f64 decimal;
		char c;
		const char *cstr;
		StringView sv;
		V2 vec2;
		V2i vec2i;
		V3 vec3;
		V4 vec4;
		Color col;
		const void *ptr;
	};
} Any;

extern const char *ANYTYPE_KIND_DISPLAYS[_ANYTYPE_COUNT];

bool any_isSignedInt(Any any);
bool any_isUnsignedInt(Any any);
bool any_isInteger(Any any);
bool any_isDecimal(Any any);
bool any_isNumber(Any any);

Any any_parseNumber(const char *cstr, char **end);

// @use    any(val1[, val2[, ...]])
// @return Any values (as follow: Any1[, Any2[, ...]]) according to given value
// @desc   -
#define any(...) VMACRO(_any, __VA_ARGS__)

// @use    any_parseNumber(cstr[, endptr])
// @return Any value base on cstr - type will be set to ANYTYPE_UNKNOWN if error occured and error will be set
// @desc   set endptr to the end of the number if set
#define any_parseNumber(...) VMACRO(_any_parseNumber, __VA_ARGS__)

// ---

Any _any_parseNumberImpl(const char *cstr, const char **endptr);

#define _any_orZero(type, arg) \
	_Generic((arg), \
		type: (arg), \
		default: (type){0})

#define _any0() 0
#define _any1(x) ({ \
	typeof(x) _x = (x); \
	_Generic(_x, \
		bool: (Any){ .type = ANYTYPE_BOOL, .boolean = _any_orZero(bool, _x) }, \
		i8: (Any){ .type = ANYTYPE_I8, .signedInt = _any_orZero(i8, _x) }, \
		i16: (Any){ .type = ANYTYPE_I16, .signedInt = _any_orZero(i16, _x) }, \
		i32: (Any){ .type = ANYTYPE_I32, .signedInt = _any_orZero(i32, _x) }, \
		i64: (Any){ .type = ANYTYPE_I64, .signedInt = _any_orZero(i64, _x) }, \
		u8: (Any){ .type = ANYTYPE_U8, .unsignedInt= _any_orZero(u8, _x) }, \
		u16: (Any){ .type = ANYTYPE_U16, .unsignedInt = _any_orZero(u16, _x) }, \
		u32: (Any){ .type = ANYTYPE_U32, .unsignedInt = _any_orZero(u32, _x) }, \
		u64: (Any){ .type = ANYTYPE_U64, .unsignedInt = _any_orZero(u64, _x) }, \
		f32: (Any){ .type = ANYTYPE_F32, .decimal = _any_orZero(f32, _x) }, \
		f64: (Any){ .type = ANYTYPE_F64, .decimal = _any_orZero(f64, _x) }, \
		char: (Any){ .type = ANYTYPE_CHAR, .c = _any_orZero(char, _x) }, \
		char*: (Any){ .type = ANYTYPE_CSTR, .cstr = _any_orZero(char*, _x) }, \
		const char*: (Any){ .type = ANYTYPE_CSTR, .cstr = _any_orZero(const char*, _x) }, \
		StringView: (Any){ .type = ANYTYPE_SV, .sv = _any_orZero(StringView, _x) }, \
		V2: (Any){ .type = ANYTYPE_V2, .vec2 = _any_orZero(V2, _x) }, \
		V2i: (Any){ .type = ANYTYPE_V2I, .vec2i = _any_orZero(V2i, _x) }, \
		V3: (Any){ .type = ANYTYPE_V3, .vec3 = _any_orZero(V3, _x) }, \
		V4: (Any){ .type = ANYTYPE_V4, .vec4 = _any_orZero(V4, _x) }, \
		Color: (Any){ .type = ANYTYPE_COLOR, .col = _any_orZero(Color, _x) }, \
		Any: _x, \
		default: type_isPtr(_x) ?  \
			(Any){ .type = ANYTYPE_PTR, .ptr = *(void **)&_x } \
			: (Any){ .type = ANYTYPE_UNKNOWN } ); })

#define _any2(x, y) _any1(x), _any1(y)
#define _any3(x, ...) _any1(x), _any2(__VA_ARGS__)
#define _any4(x, ...) _any1(x), _any3(__VA_ARGS__)
#define _any5(x, ...) _any1(x), _any4(__VA_ARGS__)
#define _any6(x, ...) _any1(x), _any5(__VA_ARGS__)
#define _any7(x, ...) _any1(x), _any6(__VA_ARGS__)
#define _any8(x, ...) _any1(x), _any7(__VA_ARGS__)

#define _any_parseNumber1(cstr) _any_parseNumberImpl((cstr), nullptr)
#define _any_parseNumber2(cstr, endptr) _any_parseNumberImpl((cstr), (endptr))
