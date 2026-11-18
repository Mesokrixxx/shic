#include "shic/container/string.h"
#include "shic/math/util.h"
#include <string.h>

void _string_initImpl(Allocator *allocator, String *strptr, const char *cstr, u64 reserve) {
	u64 cstrlen = cstr ? strlen(cstr) : 0;
	
	reserve = math_max(reserve, cstrlen + 1);
	dynlist_init(allocator, strptr, reserve);
	if (cstrlen) {
		memcpy(*strptr, cstr, cstrlen);
		string_lengthSet(*strptr, cstrlen);
	}
	(*strptr)[cstrlen] = 0;
}

void _string_appendImpl(String *strptr, char c, u64 n) {
	u64 stringLen = string_length(*strptr);
	u64 totLen = stringLen + n;
	
	string_reserve(strptr, totLen + 1);
	
	String str = *strptr;

	for (u64 i = 0; i < n; i++)
		str[stringLen + i] = c;
	str[totLen] = 0;
	string_lengthSet(str, totLen);
}

void _string_appendCstrImpl(String *strptr, const char *cstr, u64 n) {
	u64 stringLen = string_length(*strptr);
	u64 toCopy = math_min(strlen(cstr), n);
	u64 totLen = stringLen + toCopy;
	
	string_reserve(strptr, totLen + 1);

	String str = *strptr;

	for (u64 i = 0; i < toCopy; i++)
		str[stringLen + i] = cstr[i];
	str[totLen] = 0;
	string_lengthSet(str, totLen);
}

void string_clear(String str) {
	dynlist_clear(str);
	str[0] = 0;
}
