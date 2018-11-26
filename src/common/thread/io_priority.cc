#include <unistd.h>

#ifdef __linux__
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <cerrno>
#endif

#include <algorithm>

#include "io_priority.h"

pid_t flame_gettid() {
#ifdef __linux__
    return syscall(SYS_gettid);
#else
    return -ENOSYS;
#endif
}

int flame_ioprio_set(int whence, int who, int ioprio) {
#ifdef __linux__
    return syscall(SYS_ioprio_set, whence, who, ioprio);
#else
    return -ENOSYS;
#endif
}

int flame_ioprio_string_to_class(const std::string& s) {
    std::string l = s;
    std::transform(l.begin(), l.end(), l.begin(), ::tolower);

    if (l == "idle")
        return IOPRIO_CLASS_IDLE;
    if (l == "be" || l == "besteffort" || l == "best effort")
        return IOPRIO_CLASS_BE;
    if (l == "rt" || l == "realtime" || l == "real time")
        return IOPRIO_CLASS_RT;
    return -EINVAL;
}
