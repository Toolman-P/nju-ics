#ifndef __FS_H__
#define __FS_H__

#include <common.h>

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

int open(const char *filename);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_read(int fd, void *buf, size_t len);
size_t close(int fd);
size_t read(int fd, void *buf, size_t len);
size_t write(int fd, const void *buf, size_t len);
size_t lseek(int fd, size_t offset, int whence);

#endif
