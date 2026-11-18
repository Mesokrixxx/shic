#include "shic/types/stringview.h"
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
