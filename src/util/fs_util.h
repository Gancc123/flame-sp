#ifndef FLAME_UTIL_FS_H
#define FLAME_UTIL_FS_H

#include <string>

extern std::string path_join(const std::string &base, const std::string &path);
extern bool file_existed(const std::string &path);
extern bool file_readable(const std::string &path);
extern bool file_writable(const std::string &path);
extern bool file_executable(const std::string &path);
extern int dir_create(const std::string &dir);

#endif // FLAME_UTIL_FS_H