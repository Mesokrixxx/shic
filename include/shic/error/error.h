#pragma once

#include "shic/types/any.h"
#include "shic/core/macros.h"

const char *error_get();

#define error_set(fmt, ...) _error_setImpl((fmt), (Any[]){ any(__VA_ARGS__) }, NARG(__VA_ARGS__))

// @use    error_setViaErrno([prefix])
// @return -
// @desc   will format string as follow 'prefix: error' if prefix is set, 'error' otherwise
#define error_setViaErrno(...) VMACRO(_error_setViaErrno, __VA_ARGS__)

// ---

void _error_setViaErrnoImpl(const char *prefix);
void _error_setImpl(const char *fmt, const Any *args, u64 argCount);

#define _error_setViaErrno0() _error_setViaErrnoImpl(nullptr)
#define _error_setViaErrno1(prefix) _error_setViaErrnoImpl(prefix)
