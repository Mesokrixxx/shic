#pragma once

#include "shic/types/any.h"
#include "shic/core/macros.h"
#include <errno.h>

const char *error_get();


#define error_set(fmt, ...) _error_setImpl((fmt), (Any[]){ any(__VA_ARGS__) }, NARG(__VA_ARGS__))
#define error_setViaErrno(...) error_setViaCode(errno __VA_OPT__(,) __VA_ARGS__)

// @use    error_setViaCode(code[, prefix])
// @return -
// @desc   will format string as follow 'prefix: error' if prefix is set, 'error' otherwise
#define error_setViaCode(...) VMACRO(_error_setViaCode, __VA_ARGS__)

// ---

void _error_setImpl(const char *fmt, const Any *args, u64 argCount);
void _error_setViaCodeImpl(int code, const char *prefix);

#define _error_setViaCode1(code) _error_setViaCodeImpl((code), nullptr)
#define _error_setViaCode2(code, prefix) _error_setViaCodeImpl((code), (prefix))
