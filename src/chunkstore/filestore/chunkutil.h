#ifndef CHUNKSTORE_CHUNKUTIL_H
#define CHUNKSTORE_CHUNKUTIL_H

#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>

#define BLOCK_SIZE      (1U << 12)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define panic(fmt, args...)         \
({                      \
    fprintf(stderr, "PANIC: " fmt, ##args); \
    abort();                \
})

namespace util {
    extern mode_t def_dmode;
    extern mode_t def_fmode;

    void *xzalloc(size_t size);
    void *xmalloc(size_t size);
    void *xrealloc(void *ptr, size_t size);
    void *xcalloc(size_t nmemb, size_t size);
    void *xvalloc(size_t size);

    int xaccess(const char *path_name, int mode);
    int xrename(const char *oldpath, const char *newpath);
    int xremove(const char *filename);
    int xmkdir(const char *pathname, mode_t mode);
    int remove_dir(const char* pathname);
    int xfallocate(int fd, int mode, off_t offset, off_t length);
    int xftruncate(int fd, off_t length);
    int xstatfs(const char *path, struct statfs *buff);

    ssize_t get_xattr(const char *path, const char *name, void *value, size_t size);
    int set_xattr(const char *path, const char *name, const void *value, size_t size, int flags);

    ssize_t _read(int fd, void *buf, size_t len);
    ssize_t _write(int fd, const void *buf, size_t len);
    ssize_t xread(int fd, void *buf, size_t count);
    ssize_t xwrite(int fd, const void *buf, size_t count);
    ssize_t _pread(int fd, void *buf, size_t len, off_t offset);
    ssize_t _pwrite(int fd, const void *buf, size_t len, off_t offset);
    ssize_t xpread(int fd, void *buf, size_t count, off_t offset);
    ssize_t xpwrite(int fd, const void *buf, size_t count, off_t offset);
    int prealloc(int fd, uint64_t size);
    void find_zero_blocks(const void *buf, uint64_t *poffset, uint32_t *plen);
}

#endif