#include "shic/error/error.h"
#include "shic/core/fnattrib.h"
#include "shic/io/print.h"
#include "shic/memory/mallocator.h"
#include <string.h>

String errStr;

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
	string_init(allocator(&g_mallocator), &errStr, nullptr, 512);
}

static execOnExit void destructor() {
	string_destroy(errStr);
}

const char *error_get() {
	return errStr;
}

void _error_setViaCodeImpl(int code, const char *prefix) {
	error_set("{}{}{}", 
		prefix ? prefix : "", 
		prefix ? ": " : "", 
		strerror(code));
}

void _error_setImpl(const char *fmt, const Any* args, u64 argCount) {
	string_clear(errStr);
	vsprint(&errStr, fmt, args, argCount);
}
