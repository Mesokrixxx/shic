#pragma once

#include "shic/debug/log.h"
#include <stdlib.h>

#define UNREACHABLE 0
#define ABORTED 0

#define assert(cond, fmt, ...) \
	do { \
		if (cond) \
			break ; \
		error("assertion failed '{}'", #cond); \
		error(fmt __VA_OPT__(,) __VA_ARGS__); \
		exit(1); \
	} while (0)

#undef unreachable
#define unreachable() assert(UNREACHABLE, "unreachable")

#define abort(fmt, ...) assert(ABORTED, (fmt) __VA_OPT__(,) __VA_ARGS__)
