#include "shic/io/file.h"
#include "shic/error/error.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

File *file_create(Allocator *allocator, const char *path) {
	String pathStr, contentStr;
	string_init(allocator, &pathStr, path, path ? 0 : 64);
	string_init(allocator, &contentStr, nullptr, 256);

	File *file = allocator_alloc(allocator, sizeof(File));
	*file = (File){
		.allocator = allocator,
		.path = pathStr,
		.content = contentStr
	};

	return file;
}

void file_pathSet(File *file, const char *path) {
	string_set(&file->path, path);
}

bool file_load(File *file) {
	int fd = open(file->path, O_RDONLY);
	if (fd < 0) {
		error_setViaErrno("open");
		return false;
	}

	struct stat st;
	if (fstat(fd, &st) < 0) {
		error_setViaErrno("fstat");
		close(fd);
		return false;
	}

	u64 fileSize = st.st_size;
	u64 fileOff = 0;

	string_reserve(&file->content, fileSize + 1);
	while (fileOff < fileSize) {
		ssize_t readRet = read(fd, file->content + fileOff, fileSize);
		if (readRet < 0) {
			error_setViaErrno("read");
			close(fd);
			return false;
		}
		if (!readRet)
			break ;
		fileOff += readRet;
		file->content[fileOff] = 0;
	}

	close(fd);
	return true;
}

void file_destroy(File *file) {
	if (!file)
		return ;
	string_destroy(file->content);
	string_destroy(file->path);
	allocator_free(file->allocator, file);
}
