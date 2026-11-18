#pragma once

#include "shic/memory/allocator.h"

typedef struct {
	Allocator _base;
	u64 reserved;
} Mallocator;

extern Mallocator g_mallocator;
