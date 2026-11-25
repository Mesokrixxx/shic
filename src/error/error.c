#include "shic/error/error.h"
#include "shic/core/fnattrib.h"
#include "shic/io/print.h"
#include "shic/memory/mallocator.h"
#include <string.h>

String errStr1;
String errStr2;
u8 errIdx;

#ifdef WINDOWS

String tmpErrnoBuf;

static execOnInit void win_constructor() {
	string_init(allocator(&g_mallocator), &tmpErrnoBuf, nullptr, 1024);
}

static execOnExit void win_destructor() {
	string_destroy(tmpErrnoBuf);
}

# define strerror(code) ({ \
		strerror_s(tmpErrnoBuf, string_capacity(tmpErrnoBuf), (code)); \
		(const char *)tmpErrnoBuf; })

#endif // WINDOWS

static execOnInit void constructor() {
	string_init(allocator(&g_mallocator), &errStr1, nullptr, 512);
	string_init(allocator(&g_mallocator), &errStr2, nullptr, 512);
	errIdx = 0;
}

static execOnExit void destructor() {
	string_destroy(errStr1);
	string_destroy(errStr2);
}

const char *error_get() {
	return errIdx ? errStr1 : errStr2;
}

void _error_setViaCodeImpl(int code, const char *prefix) {
	error_set("{}{}{}", 
		prefix ? prefix : "", 
		prefix ? ": " : "", 
		strerror(code));
}

void _error_setImpl(const char *fmt, const Any* args, u64 argCount) {
	String dest = errIdx ? errStr2 : errStr1;

	string_clear(dest);
	vsprint(&dest, fmt, args, argCount);
	errIdx = 1 - errIdx;
}
