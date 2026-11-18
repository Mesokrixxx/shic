#pragma once

#include "shic/container/string.h"
#include "shic/core/macros.h"

typedef struct {
	Allocator *allocator;
	String path;
	String content;
} File;

File *file_create(Allocator *allocator, const char *path);
void file_pathSet(File *file, const char *path);
bool file_load(File *file);
void file_destroy(File *file);
