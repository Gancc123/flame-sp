#ifndef FLAME_COMMON_IO_PRIORITY_H
#define FLAME_COMMON_IO_PRIORITY_H

#include <sys/types.h>

#include <string>

extern pid_t flame_gettid();

#ifndef IOPRIO_WHO_PROCESS
    #define IOPRIO_WHO_PROCESS 1
#endif

#ifndef IOPRIO_PRIO_VALUE
    #define IOPRIO_CLASS_SHIFT 13
    #define IOPRIO_PRIO_VALUE(class, data) \
		(((class) << IOPRIO_CLASS_SHIFT) | (data))
#endif

#ifndef IOPRIO_CLASS_RT
    #define IOPRIO_CLASS_RT 1
#endif

#ifndef IOPRIO_CLASS_BE
    #define IOPRIO_CLASS_BE 2
#endif

#ifndef IOPRIO_CLASS_IDLE
    #define IOPRIO_CLASS_IDLE 3
#endif

extern int flame_ioprio_set(int whence, int who, int ioprio);

extern int flame_ioprio_string_to_class(const std::string& s);

#endif //FLAME_COMMON_IO_PRIORITY_H
