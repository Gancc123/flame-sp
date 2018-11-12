#ifndef FLAME_COMMON_LOGGER_H
#define FLAME_COMMON_LOGGER_H

#include <cassert>
#include <string>
#include <iostream>
#include <memory>

#include "spdlog/spdlog.h"

#include "common/clog.h"

//using namespace std;
namespace spd = spdlog;

namespace flame {

enum LoggerType {
    BASIC,
    ROTATING,
    DAILY,
    CONSOLE
};

enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
    OFF
};

class Logger {
public:
    //* 设置全局模式
    static inline void set_global_pattern(const std::string& format_string) {
        spd::set_pattern(format_string);
    }

    //* 设置全局级别
    static inline void set_global_level(int level) {
        spd::set_level(Logger::level_translate(level));
    }

    //* 设置全局同步模式
    static inline void set_global_async_mode(size_t qsize) {
        spd::set_async_mode(qsize);
    }

    //* 设置全局异步模式
    static inline void set_global_sync_mode() {
        spd::set_sync_mode();
    }

    Logger(const std::string& name) 
        : name_(name), logger_(spd::stdout_color_mt(name)) {}
    Logger(const std::string& name, int type, const std::string& path);
    virtual ~Logger() { close(); }

    bool valid() { return logger_ != nullptr; }
    void reopen(int type, const std::string& path);
    void close() { spd::drop(name_); logger_ = nullptr; }
    std::string get_name() const { return name_; }

    inline void set_pattern(const std::string& format_string) {
        logger_->set_pattern(format_string);
    }

    inline void set_level(int level) {
        logger_->set_level(Logger::level_translate(level));
    }

    inline void flush(){
        logger_->flush();
    }

    //* 设置写日志级别
    inline void flush_on(int level){
        logger_->flush_on(Logger::level_translate(level));
    }

    template<typename Arg1, typename... Args>
    inline void info(const char *fmt, const Arg1 &arg1, const Args &... args) {
        logger_->info(fmt, arg1, args...);
    }

    template<typename Arg1, typename... Args>
    inline void warn(const char *fmt, const Arg1 &arg1, const Args &... args) {
        logger_->warn(fmt, arg1, args...);
    }

    template<typename Arg1, typename... Args>
    inline void error(const char *fmt, const Arg1 &arg1, const Args &... args) {
        logger_->error(fmt, arg1, args...);
    }

    template<typename Arg1, typename... Args>
    inline void critical(const char *fmt, const Arg1 &arg1, const Args &... args) {
        logger_->critical(fmt, arg1, args...);
    }

    template<typename Arg1, typename... Args>
    inline void debug(const char* fmt, const Arg1 &arg1, const Args &... args) {
        logger_->debug(fmt, arg1, args...);
    }

    template<typename Arg1, typename... Args>
    inline void trace(const char *fmt, const Arg1 &arg1, const Args &... args) {
        logger_->trace(fmt, arg1, args...);
    }

    template<typename ...Args>
    inline void info(const wchar_t* fmt, const Args &... args) {
        logger_->info(fmt, args...);
    }

    template<typename ...Args>
    inline void warn(const wchar_t* fmt, const Args &... args) {
        logger_->warn(fmt, args...);
    }

    template<typename ...Args>
    inline void error(const wchar_t *fmt, const Args &... args) {
        logger_->error(fmt, args...);
    }

    template<typename ...Args>
    inline void critical(const wchar_t * fmt, const Args &... args) {
        logger_->critical(fmt, args...);
    }

    template<typename ...Args>
    inline void debug(const wchar_t *fmt, const Args &... args) {
        logger_->debug(fmt, args...);
    }

    template<typename ...Args>
    inline void trace(const wchar_t *fmt, const Args &... args) {
        logger_->trace(fmt, args...);
    }

    template<typename T>
    inline void info(const T& msg) {
        logger_->info(msg);
    }

    template<typename T>
    inline void warn(const T& msg) {
        logger_->warn(msg);
    }

    template<typename T>
    inline void error(const T& msg) {
        logger_->error(msg);
    }

    template<typename T>
    inline void critical(const T &msg) {
        logger_->critical(msg);
    }

    template<typename T>
    inline void debug(const T& msg) {
        logger_->debug(msg);
    }

    template<typename T>
    inline void trace(const T& msg) {
        logger_->trace(msg);
    }

private:
    std::string name_;
    std::shared_ptr<spd::logger> logger_;

    static spd::level::level_enum level_translate(int level) {
        switch(level) {
        case LogLevel::TRACE:   return spd::level::trace;
        case LogLevel::DEBUG:   return spd::level::debug;
        case LogLevel::INFO:    return spd::level::info;
        case LogLevel::WARN:    return spd::level::warn;
        case LogLevel::ERROR:   return spd::level::err;
        case LogLevel::CRITICAL: return spd::level::critical;
        case LogLevel::OFF:     return spd::level::off;
        default: return spd::level::info;
        }
    }
}; // class Logger

} // namespace flame

#endif // FLAME_COMMON_LOGGER_H
