#include "shic/io/file.h"
#include "shic/error/error.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef WINDOWS
# include <io.h>

# define read _read
# define close _close

static bool openFile(int *dest, const char *path) {
	int err = _sopen_s(dest, path, _O_RDONLY | _O_BINARY, _SH_DENYNO, 0);
	if (err) {
		error_setViaCode(err, "open");
		return false;
	}
	return true;
}
#else
# include <unistd.h>

static bool openFile(int *dest, const char *path) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		error_setViaErrno("open");
		return false;
	}
	
	*dest = fd;
	return true;
}
#endif // WINDOWS

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
	int fd;

	if (!openFile(&fd, file->path))
		return false;

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
		i64 readRet = read(fd, file->content + fileOff, fileSize);
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
