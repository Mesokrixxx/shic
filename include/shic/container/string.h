#pragma once

#include "shic/container/dynlist.h"

typedef Dynlist(char) String;

void string_clear(String str);

#define string_reserve dynlist_reserve
#define string_lengthSet dynlist_sizeSet
#define string_length dynlist_size
#define string_capacity dynlist_capacity
#define string_forEach dynlist_forEach
#define string_destroy dynlist_destroy

// @use    string_init(allocator, strptr, cstr[, reserve])
// @return -
// @desc   will reserve 'reserve' chars, strlen(cstr) + 1 by default
#define string_init(...) VMACRO(_string_init, __VA_ARGS__)

// @use    string_append(strptr, c[, n])
// @return -
// @desc   c will be added n times, 1 by default
#define string_append(...) VMACRO(_string_append, __VA_ARGS__)

// @use    string_appendCstr(strptr, cstr[, n])
// @return -
// @desc   n chars of cstr will be added, UINT64_MAX by default
#define string_appendCstr(...) VMACRO(_string_appendCstr, __VA_ARGS__)

#define string_set(...) VMACRO(_string_set, __VA_ARGS__)

// ---

void _string_initImpl(Allocator *allocator, String *strptr, const char *cstr, u64 reserve);
void _string_appendImpl(String *strptr, char c, u64 n);
void _string_appendCstrImpl(String *strptr, const char *cstr, u64 n);

#define _string_init3(allocator, strptr, cstr) _string_initImpl((allocator), (strptr), (cstr), 0)
#define _string_init4(allocator, strptr, cstr, reserve) _string_initImpl((allocator), (strptr), (cstr), (reserve))
#define _string_append2(strptr, c) _string_appendImpl((strptr), (c), 1)
#define _string_append3(strptr, c, n) _string_appendImpl((strptr), (c), (n))
#define _string_appendCstr2(strptr, cstr) _string_appendCstrImpl((strptr), (cstr), UINT64_MAX)
#define _string_appendCstr3(strptr, cstr, n) _string_appendCstrImpl((strptr), (cstr), (n))
#define _string_set2(strptr, cstr) do { String *str = (strptr); string_clear(*str); _string_appendCstr2(str, (cstr)); } while (0)
#define _string_set3(strptr, cstr, n) do { String *str = (strptr); string_clear(*str); _string_appendCstr3(str, (cstr), (n)); } while (0)