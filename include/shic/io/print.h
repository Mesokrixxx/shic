#pragma once

#include "shic/types/alias.h"
#include "shic/types/any.h"
#include "shic/container/string.h"
#include "shic/core/macros.h"

#define print(fmt, ...) vfprint(1, (fmt), (Any[]){ any(__VA_ARGS__) }, NARG(__VA_ARGS__))
#define fprint(fd, fmt, ...) vfprint((fd), (fmt), (Any[]){ any(__VA_ARGS__) }, NARG(__VA_ARGS__))
#define sprint(destptr, fmt, ...) vsprint((destptr), (fmt), (Any[]){ any(__VA_ARGS__) }, NARG(__VA_ARGS__))

u64 vfprint(int fd, const char *fmt, const Any *args, u64 argCount);
u64 vsprint(String *destptr, const char *fmt, const Any *args, u64 argCount);
