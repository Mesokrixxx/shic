#pragma once

#include "shic/types/alias.h"

typedef struct {
	const char *str;
	u64 len;
} StringView;

#define STRINGVIEW(cstr) ((StringView){ .str = (cstr), .len = ARRAY_SIZE(cstr) - 1 })

StringView stringview(const char *cstr);
bool stringview_eq(StringView sv1, StringView sv2);
StringView stringview_getLine(const char *cstr);
StringView stringview_chop(const StringView sv, u64 off);
StringView stringview_rchop(const StringView sv, u64 off);
