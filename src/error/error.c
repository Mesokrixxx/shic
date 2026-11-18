#include "shic/error/error.h"
#include "shic/compiler/attribute.h"
#include "shic/io/print.h"
#include "shic/memory/mallocator.h"
#include <string.h>
#include <errno.h>

String errStr;

static attribute_onInit void constructor() {
	string_init(allocator(&g_mallocator), &errStr, nullptr, 512);
}

static attribute_onExit void destructor() {
	string_destroy(errStr);
}

const char *error_get() {
	return errStr;
}

void _error_setViaErrnoImpl(const char *prefix) {
	error_set("{}{}{}", 
		prefix ? prefix : "", 
		prefix ? ": " : "", 
		strerror(errno));
}

void _error_setImpl(const char *fmt, const Any* args, u64 argCount) {
	string_clear(errStr);
	vsprint(&errStr, fmt, args, argCount);
}
