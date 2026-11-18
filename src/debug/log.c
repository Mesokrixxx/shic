#include "shic/debug/log.h"
#include "shic/io/print.h"
#include "shic/memory/mallocator.h"
#include "shic/compiler/attribute.h"

static String g_buf;

static attribute_onInit void _initialize() {
	string_init(allocator(&g_mallocator), &g_buf, nullptr, 512);
}

static attribute_onExit void _destroy() {
	string_destroy(g_buf);
}

void _logImpl(int fd, const char *file, int line, const char *func, const char *header, const char *fmt, const Any *args, u64 argCount) {
	string_clear(g_buf);
	vsprint(&g_buf, fmt, args, argCount);
	fprint(fd, 
		"[{}][{}:{}][{}] {}{}", 
		header, file, line, func, 
		g_buf, 
		g_buf[string_length(g_buf) - 1] != '\n' ? "\n" : "");
}
