#include "fs_util.h"

std::string path_join(const std::string &base, const std::string &path) {
    size_t len = base.size() - 1, pos = 0, plen = path.size() - 1;

    while (len >= 0 && base[len] == '/')
        --len;

    while (pos < path.size() && path[pos] == '/')
        ++pos;

    while (plen >= pos && path[plen] == '/')
        --plen;

    return base.substr(0, len + 1) + '/' + path.substr(pos, plen - pos + 1);
}

bool file_existed(const std::string &path) {
    if (access(path.c_str(), F_OK) == 0)
        return true;
    return false;
}

bool file_readable(const std::string &path) {
    if (access(path.c_str(), R_OK) == 0)
        return true;
    return false;
}

bool file_writable(const std::string &path) {
    if (access(path.c_str(), W_OK) == 0)
        return true;
    return false;
}

bool file_executable(const std::string &path) {
    if (access(path.c_str(), X_OK) == 0)
        return true;
    return false;
}

int dir_create(const std::string &dir) {
    res = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (res && errno != EEXIST)
        return 1;
    return 0;
}