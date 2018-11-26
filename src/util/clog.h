#ifndef FLAME_UTIL_CLOG_H
#define FLAME_UTIL_CLOG_H

#include <iostream>
#include <cstdio>
#include <string>

inline void clog(const char * const str) {
    std::cerr << str << std::endl;
    std::cerr.flush();
}

inline void clog(const std::string &str) {
    std::cerr << str << std::endl;
    std::cerr.flush();
}

inline void dout_emergency(const char * const str) {
    std::cerr << str;
    std::cerr.flush();
}

inline void dout_emergency(const std::string &str) {
    std::cerr << str;
    std::cerr.flush();
}

inline void assert_msg__(int expression, const std::string& msg) {
    if (!expression) {
        std::cerr << msg << std::endl;
        exit(134);
    }
} 

#ifdef NDEBUG

#define dout(format, ...)
#define derr(format, ...)
#define assert_msg(exp, msg)

#else

#define dout(format, ...) fprintf(stdout, "Debug " __FILE__ "(%d): " format "\n", __LINE__, ##__VA_ARGS__)
#define derr(format, ...) fprintf(stderr, "Debug " __FILE__ "(%d): " format "\n", __LINE__, ##__VA_ARGS__)
#define assert_msg(exp, msg) assert_msg__((exp), (msg))

#endif

#endif // FLAME_UTIL_CLOG_H