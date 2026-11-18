#pragma once

#include "shic/types/any.h"

#define LOG_COLOR_RED "\e[31m"
#define LOG_COLOR_YEL "\e[33m"
#define LOG_COLOR_BLU "\e[34m"
#define LOG_COLOR_RST "\e[0m"

#define log(fmt, ...) _logStepImpl(1, LOG_COLOR_BLU "LOG" LOG_COLOR_RST, fmt __VA_OPT__(,) __VA_ARGS__)
#define warn(fmt, ...) _logStepImpl(1, LOG_COLOR_YEL "WRN" LOG_COLOR_RST, fmt __VA_OPT__(,) __VA_ARGS__)
#define error(fmt, ...) _logStepImpl(2, LOG_COLOR_RED "ERR" LOG_COLOR_RST, fmt __VA_OPT__(,) __VA_ARGS__)

// ---

#define _logStepImpl(fd, header, fmt, ...) \
	_logImpl(fd, __FILE__, __LINE__, __func__, header, fmt, (Any[]){ any(__VA_ARGS__) }, NARG(__VA_ARGS__))

void _logImpl(int fd, const char *file, int line, const char *func, const char *header, const char *fmt, const Any *args, u64 argCount);
