#include "shic/types/stringview.h"
#include "shic/math/util.h"
#include <string.h>

StringView stringview(const char *cstr) {
	return (StringView){ .str = cstr, .len = cstr ? strlen(cstr) : 0 };
}

bool stringview_eq(StringView sv1, StringView sv2) {
	return sv1.len == sv2.len && !strncmp(sv1.str, sv2.str, sv1.len);
}

StringView stringview_getLine(const char *cstr) {
	StringView sv = { .str = cstr };

	while (cstr[sv.len] && cstr[sv.len] != '\n')
		sv.len++;
	return sv;
}

StringView stringview_chop(const StringView sv, u64 off) {
	return (StringView){ .str = sv.str + off, math_max(sv.len - off, 0) };
}

StringView stringview_rchop(const StringView sv, u64 off) {
	return (StringView){ .str = sv.str, math_max(sv.len - off, 0) };
}
