#ifndef FLAME_MSG_MSG_DEF_H
#define FLAME_MSG_MSG_DEF_H

#include "acconfig.h"
#include "util/fmt.h"

namespace flame{

enum class msg_log_level_t : uint8_t{
    dead      = 0,
    critical  = 1,
    wrong     = 2,

    error     = 3,
    warn      = 4,
    info      = 5,

    debug     = 6,
    trace     = 7,
    print     = 8
};

extern msg_log_level_t g_msg_log_level;

}

#define MSG_LOG_COMMON(level, s)  l##level("msg", "%s", s)
#define MSG_KLOG_COMMON(level, s) k##level("msg", "%s", s)

#if 0 //defined(MSG_NO_LOG)
    #define ML(mct, level, format, ...) do{}while(0)
    #define MLI(mct, level, format, ...) do{}while(0)
    #define MKL(mct, level, format, ...) do{}while(0)
    #define MKLI(mct, level, format, ...) do{}while(0)
#else

    #define ML(mct, level, fmt_, ...)\
            do{\
                using flame::g_msg_log_level;\
                if((mct) && g_msg_log_level >= msg_log_level_t::level)\
                    (mct)->fct->log()->MSG_LOG_COMMON(level, \
                                    fmt::format(fmt_, ## __VA_ARGS__).c_str());\
            }while(0)

    #define MLI(mct, level, fmt_, ...)\
            do{\
                using flame::g_msg_log_level;\
                if((mct) && g_msg_log_level >= msg_log_level_t::level)\
                    (mct)->fct->log()->MSG_LOG_COMMON(level, \
                                fmt::format("{:p} " fmt_, (void *)this, \
                                                ## __VA_ARGS__).c_str());\
            }while(0)

    #define MKL(mct, level, fmt_, ...)\
            do{\
                using flame::g_msg_log_level;\
                if((mct) && g_msg_log_level >= msg_log_level_t::level)\
                    (mct)->fct->log()->MSG_KLOG_COMMON(level, \
                                    fmt::format(fmt_, ## __VA_ARGS__).c_str());\
            }while(0)

    #define MKLI(mct, level, fmt_, ...)\
            do{\
                using flame::g_msg_log_level;\
                if((mct) && flame::g_msg_log_level >= msg_log_level_t::level)\
                    (mct)->fct->log()->MSG_KLOG_COMMON(level, \
                                fmt::format("{:p} " fmt_, (void *)this, \
                                                ## __VA_ARGS__).c_str());\
            }while(0)

#endif

#endif //FLAME_MSG_MSG_DEF_H