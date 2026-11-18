#include "shic/types/any.h"
#include "shic/error/error.h"
#include "shic/math/util.h"
#include <ctype.h>

const char *ANYTYPE_KIND_DISPLAYS[] = {
#define X(enum) [CONCAT(ANYTYPE_, enum)] = #enum,
	_ANYTYPE_LIST(X)
#undef X
};

bool any_isSignedInt(Any any) {
	return any.type >= ANYTYPE_I8 && any.type <= ANYTYPE_I64;
}

bool any_isUnsignedInt(Any any) {
	return any.type >= ANYTYPE_U8 && any.type <= ANYTYPE_U64;
}

bool any_isInteger(Any any) {
	return any_isSignedInt(any) || any_isUnsignedInt(any);
}

bool any_isDecimal(Any any) {
	return any.type == ANYTYPE_F32 || any.type == ANYTYPE_F64;
}

bool any_isNumber(Any any) {
	return any_isInteger(any) || any_isDecimal(any);
}

static AnyType getIntegerType(u64 value) {
	if (value > INT64_MAX)
		return ANYTYPE_U64;
	else if (value > UINT32_MAX)
		return ANYTYPE_I64;
	else if (value > INT32_MAX)
		return ANYTYPE_U32;
	else if (value > UINT16_MAX)
		return ANYTYPE_I32;
	else if (value > INT16_MAX)
		return ANYTYPE_U16;
	else if (value > UINT8_MAX)
		return ANYTYPE_I16;
	else if (value > INT8_MAX)
		return ANYTYPE_U8;
	return ANYTYPE_I8;
}

Any _any_parseNumberImpl(const char *cstr, const char **endptr) {
	const u8 values[UINT8_MAX] = {
		['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3, ['4'] = 4, ['5'] = 5,
		['6'] = 6, ['7'] = 7, ['8'] = 8, ['9'] = 9, ['a'] = 10, ['b'] = 11,
		['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15
	};
	
	Any value = { .type = ANYTYPE_UNKNOWN };
	bool foundDot = *cstr == '.';
	i64 fractionDigits = 0;
	i64 exponent = 0;
	u8 base = 0;

	u64 i = foundDot;
	if (cstr[i] == '0') {
		if (cstr[i + 1] == 'x')
			base = 16;
		else if (cstr[i + 1] == 'b')
			base = 2;
		else if (cstr[i + 1] == 'o')
		 	base = 8;	
		if (base)
			i += 2;
	}

	if (!base)
		base = 10;

	while (true) {
		char c = cstr[i];
		if (c == '.') {
			if (foundDot) {
				error_set("found dot when already precised before");
				value.type = ANYTYPE_UNKNOWN;
				goto end;
			}
			foundDot = true;
			i++;
			continue ;
		}

		u8 val = values[(u8)c];
		if ((!val && c != '0') || val >= base)
			break ;

		value.unsignedInt = value.unsignedInt * base + val;
		if (foundDot)
			fractionDigits++;
		i++;
	}

	if (!foundDot)
		value.type = getIntegerType(value.unsignedInt);
	else
		value.type = ANYTYPE_F64;

	if (cstr[i] == 'e') {
		i++;
		
		bool neg = false;
		if (cstr[i] == '-' || cstr[i] == '+') {
			neg = cstr[i] == '-';
			i++;
		}

		while (true) {
			char c = cstr[i];
			if (!isdigit(c))
				break ;
			exponent = exponent * 10 + c - '0';
			i++;
		}

		if (neg)
			exponent = -exponent;
		value.type = ANYTYPE_F64;
	}
	
	if (value.type == ANYTYPE_F64)
		value.decimal = value.unsignedInt * math_pow10(exponent) * math_pow10(-fractionDigits);

	if (cstr[i] == 'f') {
		if (any_isInteger(value))
			value.decimal = value.unsignedInt;
		value.type = ANYTYPE_F32;
		i++;
	}

	else if (cstr[i] == 'u') {
		if (any_isDecimal(value)) {
			error_set("unsigned suffix for decimal number is invalid");
			value.type = ANYTYPE_UNKNOWN;
			goto end;
		}
		if (any_isSignedInt(value))
			value.type += 4;
		i++;
	}

end:
	if (endptr)
		*endptr = cstr + i;
	return value;
}

