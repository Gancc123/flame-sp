#ifndef FLAME_MSG_MSG_DEF_H
#define FLAME_MSG_MSG_DEF_H

#include "acconfig.h"
#include "util/fmt.h"

#define MSG_LOG_COMMON(level, s)  l##level("msg", "%s", s)
#define MSG_KLOG_COMMON(level, s) k##level("msg", "%s", s)

#if defined(MSG_NO_LOG)
    #define ML(pfct, level, format, ...) do{}while(0)
    #define MLI(pfct, level, format, ...) do{}while(0)
    #define MKL(pfct, level, format, ...) do{}while(0)
    #define MKLI(pfct, level, format, ...) do{}while(0)
#else

    #define ML(pfct, level, fmt_, ...)\
            do{\
                if(pfct) (pfct)->log()->MSG_LOG_COMMON(level, \
                                    fmt::format(fmt_, ## __VA_ARGS__).c_str());\
            }while(0)

    #define MLI(pfct, level, fmt_, ...)\
            do{\
                if(pfct) (pfct)->log()->MSG_LOG_COMMON(level, \
                                fmt::format("{:p}" fmt_, (void *)this, \
                                                ## __VA_ARGS__).c_str());\
            }while(0)

    #define MKL(pfct, level, fmt_, ...)\
            do{\
                if(pfct) (pfct)->log()->MSG_KLOG_COMMON(level, \
                                    fmt::format(fmt_, ## __VA_ARGS__).c_str());\
            }while(0)

    #define MKLI(pfct, level, fmt_, ...)\
            do{\
                if(pfct) (pfct)->log()->MSG_KLOG_COMMON(level, \
                                fmt::format("{:p}" fmt_, (void *)this, \
                                                ## __VA_ARGS__).c_str());\
            }while(0)

#endif

#endif //FLAME_MSG_MSG_DEF_H