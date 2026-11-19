#include "shic/io/print.h"
#include "shic/memory/mallocator.h"
#include "shic/debug/assert.h"
#include "shic/core/fnattrib.h"
#include "shic/debug/assert.h"
#include "shic/math/util.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#ifdef WINDOWS
# include <io.h>
# define write _write
#else
# include <unistd.h>
#endif // WINDOWS

static Dynlist(String) g_bufs;

static execOnInit void constructor() {
	dynlist_init(allocator(&g_mallocator), &g_bufs, 3);
	memset(g_bufs, 0, sizeof(String) * dynlist_capacity(g_bufs));
	for (u64 i = 0; i < 3; i++)
		string_init(allocator(&g_mallocator), &g_bufs[i], nullptr, 512);
}

static execOnExit void destructor() {
	dynlist_forEach(g_bufs, it)
		string_destroy(*it.elem);
	dynlist_destroy(g_bufs);
}

#define printAbort(fmt, ...) \
	do { string_clear(g_bufs[2]); abort(fmt __VA_OPT__(,) __VA_ARGS__); } while (0)

static void bufAdd(String *destptr, Any type) {
	char tmp[64] = {0};
	int len = 0;

	switch (type.type) {
		case (ANYTYPE_UNKNOWN):
			string_appendCstr(destptr, "???");
			break ;

		case (ANYTYPE_BOOL):
			if (type.boolean)
				string_appendCstr(destptr, "true");
			else
			 	string_appendCstr(destptr, "false");
			break ;

		case (ANYTYPE_U8):
		case (ANYTYPE_U16):
		case (ANYTYPE_U32):
		case (ANYTYPE_U64):
			len = snprintf(tmp, sizeof(tmp), "%" PRIu64, type.unsignedInt);
			break ;
		
		case (ANYTYPE_I8):
		case (ANYTYPE_I16):
		case (ANYTYPE_I32):
		case (ANYTYPE_I64):
			len = snprintf(tmp, sizeof(tmp), "%" PRIi64, type.signedInt);
			break ;

		case (ANYTYPE_F32):
		case (ANYTYPE_F64):
			len = snprintf(tmp, sizeof(tmp), "%f", type.decimal);
			break ;

		case (ANYTYPE_CHAR):
			string_append(destptr, type.c);
			break ;
		
		case (ANYTYPE_CSTR):
			if (type.cstr)
				string_appendCstr(destptr, type.cstr);
			else
				string_appendCstr(destptr, "(null)");
			break ;

		case (ANYTYPE_SV):
			string_appendCstr(destptr, type.sv.str, type.sv.len);
			break ;

		case (ANYTYPE_PTR):
			len = snprintf(tmp, sizeof(tmp), "%p", type.ptr);		
			break ;

		default:
			unreachable();
	}
	
	if (len)
		string_appendCstr(destptr, tmp, len);
}

static void handleTimes(String *destptr, const char *fmt, u64 *off, const Any **args, u64 *argCount) {
	Any what, time;
	u64 i = 0;
	
	char c = fmt[i++];
	if (c == '{' && fmt[i] == '}' && argCount) {
		what = *((*args)++);
		(*argCount)--;
		i++;
	}

	else if (c == '\'') {
		StringView sv = { .str = fmt + i };

		while (fmt[i] && fmt[i] != '\'')
			i++;
		if (!fmt[i])
			printAbort("abrupt eof when parsing times first argument");
		sv.len = fmt + i - sv.str;
		what = any(sv);
		i++;
	}
	
	else
		printAbort("times first argument must be a string or a variable: '{}'", fmt);

	c = fmt[i++];
	if (c == '{' && fmt[i] == '}' && *argCount) {
		time = *((*args)++);

		if (!any_isNumber(time))
			printAbort("times second argument must be a number: '{}'", time);
		(*argCount)--;
		i++;
	}

	else if (isdigit(c)) {
		const char *end;

		time = any_parseNumber(fmt + i - 1, &end);
		if (time.type == ANYTYPE_UNKNOWN)
			printAbort("times second argument must be a valid number: '{}'", fmt);
		i = end - fmt;
	}

	else
		printAbort("times second argument must be a number: '{}'", fmt);

	if (fmt[i] != '}')
		printAbort("unexpected character at the end of times formattage: '{}'", fmt[i]);

	if (any_isSignedInt(time))
		time.unsignedInt = math_max(0, time.signedInt);
	else if (any_isDecimal(time))
		time.unsignedInt = time.decimal;

	for (u64 j = 0; j < time.unsignedInt; j++)
		bufAdd(destptr, what);

	*off += i + 1;
}

static void bufFillWithFmt(String *destptr, const char *fmt, const Any *args, u64 argCount) {
	for (u64 i = 0; fmt[i];) {
		if (fmt[i] == '{') {
			i++;

			char c = fmt[i++];
			switch (c) {
				case ('}'): 
					if (argCount) { 
						bufAdd(destptr, *args++); 
						argCount--;
					}
					else
						string_append(destptr, c);
					break ;

				case ('t'):
					handleTimes(destptr, fmt + i, &i, &args, &argCount);
					break ;

				default:
					string_append(destptr, '{');
					string_append(destptr, c);
					break ;
			}
			continue ;
		}
		string_append(destptr, fmt[i++]);
	}
}

u64 vfprint(int fd, const char *fmt, const Any *args, u64 argCount) {
	u64 oCap = dynlist_capacity(g_bufs);

	if (dynlist_reserve(&g_bufs, fd + 1))
		memset(g_bufs + oCap, 0, sizeof(String) * (dynlist_capacity(g_bufs) - oCap));
	
	if (!g_bufs[fd])
		string_init(dynlist_allocator(g_bufs), &g_bufs[fd], nullptr, 512);

	String dest = g_bufs[fd];
	u64 initialSize = string_length(dest);
	
	bufFillWithFmt(&dest, fmt, args, argCount);
	
	u64 addedToBuf = string_length(dest) - initialSize;

	const char *nl = strrchr(dest, '\n');
	if (nl) {
		u64 toWrite = nl - dest + 1;
		u64 remaining = string_length(dest) - toWrite;
		
		write(fd, dest, toWrite);
		memmove(dest, nl + 1, remaining);
		dest[remaining] = 0;
		string_lengthSet(dest, remaining);
	}
	
	return addedToBuf;
}

u64 vsprint(String *destptr, const char *fmt, const Any *args, u64 argCount) {
	u64 initialSize = string_length(*destptr);
	
	bufFillWithFmt(destptr, fmt, args, argCount);
	return string_length(*destptr) - initialSize;
}
