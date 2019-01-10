#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/xattr.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/vfs.h>
#include <sys/statfs.h>
#include <time.h>
#include <string.h>
#include <malloc.h>

#include "chunkstore/filestore/chunkutil.h"

mode_t util::def_dmode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP;
mode_t util::def_fmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

/*
**alloc memory and init by zero
 */
void* util::xzalloc(size_t size) {
    return xcalloc(1, size);
}

/*
**malloc: alloc memory but don't init
 */
void* util::xmalloc(size_t size) {
    void *ret = malloc(size);
    if(unlikely(!ret))
        panic("Out of memory");

    return ret;
}

/*
**realloc
 */
void* util::xrealloc(void *ptr, size_t size) {
    errno = 0;
    void *ret = realloc(ptr, size);
    if(unlikely(errno == ENOMEM))
        panic("Out of memory");

    return ret;
}

/*
**alloc memory and init by zero
 */
void* util::xcalloc(size_t nmemb, size_t size) {
    void *ret = calloc(nmemb, size);
    if(unlikely(!ret))
        panic("Out of memory");

    return ret;
}
/*
**The memory address will be a multiple of the page size.
*/
void* util::xvalloc(size_t size) {
    void *ret = valloc(size);
    if(unlikely(!ret))
        panic("Out of memory");

    memset(ret, 0, size);
    return ret;
}
/*
**preallocte the whole object
*/
int util::prealloc(int fd, uint64_t size) {
    int ret = xfallocate(fd, 0, 0, size);
    if(ret < 0){
        if(errno != ENOSYS && errno != EOPNOTSUPP) {
            return ret;
        }

        return xftruncate(fd, size);
    }

    return 0;
}

int util::xaccess(const char *pathname, int mode) {
    if(access(pathname, mode) != 0) {
        return errno;
    }

    return 0;
}

int util::xmkdir(const char* pathname, mode_t mode) {
    if(mkdir(pathname, mode) < 0){
        return errno;
    }
    return 0;
}

int util::remove_dir(const char* pathname) {
    DIR *dir;
    struct dirent* dir_info;
    struct stat st;
    char subfile[512];

    if(dir = opendir(pathname)) {
        while(dir_info = readdir(dir)) {
            if(strcmp(dir_info->d_name, ".") == 0 || strcmp(dir_info->d_name, "..") == 0) {
                continue;
            }

            snprintf(subfile, sizeof(subfile), "%s/%s", pathname, dir_info->d_name);

            if(stat(subfile, &st) == 0) {
                if(S_ISREG(st.st_mode))
                    remove(subfile);
                else if(S_ISDIR(st.st_mode)) {
                    if(util::remove_dir(subfile) != 0) {
                        return -1;
                    }
                    util::xremove(subfile);
                }
            }
        }
        closedir(dir);
        util::xremove(pathname);
        return 0;
    } else {
        closedir(dir);
        return -1;
    }
}

int util::xremove(const char *filename) {
    int ret;

    do {
        ret = remove(filename);
    } while(unlikely(ret < 0) && (errno == EAGAIN || errno == EINTR));

    return ret;
}

int util::xrename(const char *oldpath, const char *newpath) {
    int ret;
    do {
        ret = rename(oldpath, newpath);
    } while(unlikely(ret < 0) && (errno == EAGAIN || errno == EINTR));

    return ret;
}

int util::xfallocate(int fd, int mode, off_t offset, off_t len) {
    int ret;
    do {
        ret = fallocate(fd, mode, offset, len);
    } while(unlikely(ret < 0) && (errno == EAGAIN || errno == EINTR));

    return ret;
}

int util::xftruncate(int fd, off_t length) {
    int ret;
    do {
        ret = ftruncate(fd, length);
    } while(unlikely(ret < 0) && (errno == EAGAIN || errno == EINTR));

    return ret;
}

ssize_t util::_read(int fd, void *buf, size_t len) {
    ssize_t nr;
    while(true){
        nr = read(fd, buf, len);
        if(unlikely(nr < 0) && (errno == EAGAIN || errno == EINTR)) {
            continue;
        }

        return nr;
    }
}

ssize_t util::xread(int fd, void *buf, size_t count) {
    char *p = (char *)buf;
    ssize_t total = 0;
    while(count > 0) {
        ssize_t loaded = _read(fd, p, count);
        if(unlikely(loaded < 0))
            return -1;
        if(unlikely(loaded == 0))
            return total;

        count -= loaded;
        p += loaded;
        total += loaded;
    }

    return total;
}

ssize_t util::_write(int fd, const void *buf, size_t len) {
    ssize_t nr;
    while(true){
        nr = write(fd, buf, len);
        if(unlikely(nr < 0) && (errno == EAGAIN || errno == EINTR)){
            continue;
        }

        return nr;
    }
}

ssize_t util::xwrite(int fd, const void *buf, size_t count) {
    const char *p = (const char *)buf;
    ssize_t  total = 0;
    while(count > 0) {
        ssize_t  written = _write(fd, p, count);
        if(unlikely(written < 0))
            return -1;
        if(unlikely(!written)) {
            errno = ENOSPC;
            return -1;
        }
        count -= written;
        p += written;
        total += written;
    }

    return total;
}

ssize_t util::_pread(int fd, void *buf, size_t len, off_t offset) {
    ssize_t nr;
    while(true){
        nr = pread(fd, buf, len, offset);
        if(unlikely(nr < 0) && (errno == EAGAIN || errno == EINTR))
            continue;

        return nr;
    }
}

ssize_t util::_pwrite(int fd, const void *buf, size_t len, off_t offset) {
    ssize_t  nr;
    while(true) {
        nr = pwrite(fd, buf, len, offset);
        if(unlikely(nr < 0) && (errno == EAGAIN || errno == EINTR))
            continue;

        return nr;
    }
}

ssize_t util::xpread(int fd, void *buf, size_t count, off_t offset) {
    char *p = (char *)buf;
    ssize_t total = 0;
    while(count > 0) {
        ssize_t loaded = _pread(fd, p, count, offset);
        if(unlikely(loaded < 0))
            return -1;

        if(unlikely(loaded == 0))
            return total;

        count -= loaded;
        p += loaded;
        total += loaded;
        offset += loaded;
    }

    return total;
}

ssize_t util::xpwrite(int fd, const void *buf, size_t count, off_t offset) {
    const char *p = (char *)buf;
    ssize_t total = 0;

    while(count > 0) {
        ssize_t written = _pwrite(fd, p, count, offset);
        if(unlikely(written < 0))
            return -1;

        if(unlikely(!written)) {
            errno = ENOSPC;
            return -1;
        }

        count -= written;
        p += written;
        total += written;
        offset += written;
    }

    return total;
}

void util::find_zero_blocks(const void *buf, uint64_t *poffset, uint32_t *plen) {
    const uint8_t zero[BLOCK_SIZE] = {0};
    const char *p = (const char *)buf;
    uint64_t start = *poffset;
    uint64_t offset = 0;
    uint32_t len = *plen;

    while(len >= BLOCK_SIZE) {
        size_t size = BLOCK_SIZE - (start + offset) % BLOCK_SIZE;
        if(memcmp(p + offset, zero, size) != 0)
            break;

        offset += size;
        len -= size;
    }

    while(len >= BLOCK_SIZE) {
        size_t size = (start + offset + len) % BLOCK_SIZE;
        if(size == 0){
            size = BLOCK_SIZE;
        }
        if(memcmp(p + offset + len - size, zero, size) != 0)
            break;

        len -= size;
    }

    *plen = len;
    *poffset = start + offset;
}

ssize_t util::get_xattr(const char *path, const char *name, void *value, size_t size) {
    ssize_t ret;
    do {
        ret = getxattr(path, name, value, size);
    } while(unlikely(ret < 0) && (errno == E2BIG || errno == ERANGE));

    return ret;
}

int util::set_xattr(const char *path, const char *name, const void *value, size_t size, int flags) {
    int ret;
    do {
        ret = setxattr(path, name, value, size, flags);
    }while(unlikely(ret < 0) && (errno == EDQUOT || errno == ENOSPC || errno == EPERM));

    return ret;
}

int util::xstatfs(const char *path, struct statfs *buff) {
    if(statfs(path, buff) != 0)
        return errno;
    
    return 0;
}