#ifndef FLAME_INCLUDE_LOG_H
#define FLAME_INCLUDE_LOG_H

#include <cstdio>
#include <string>
#include <atomic>

#define MAX_LOG_LEN 512
// #define log(level, module, fmt, arg...) plog((level), (module), __FILE__, __LINE__, (fmt), ##arg)

/**
 * Normal Log
 */
#define dead(module, fmt, arg...) plog(0, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define critical(module, fmt, arg...) plog(1, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define wrong(module, fmt, arg...) plog(2, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)

#define error(module, fmt, arg...) plog(3, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define warn(module, fmt, arg...) plog(4, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define info(module, fmt, arg...) plog(5, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)

#define debug(module, fmt, arg...) plog(6, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define trace(module, fmt, arg...) plog(7, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define print(module, fmt, arg...) plog(8, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)

/**
 * Key Path Log
 */
#define kdead(module, fmt, arg...) klog(0, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define kcritical(module, fmt, arg...) klog(1, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define kwrong(module, fmt, arg...) klog(2, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)

#define kerror(module, fmt, arg...) klog(3, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define kwarn(module, fmt, arg...) klog(4, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define kinfo(module, fmt, arg...) klog(5, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)

#define kdebug(module, fmt, arg...) klog(6, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define ktrace(module, fmt, arg...) klog(7, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)
#define kprint(module, fmt, arg...) klog(8, (module), __FILE__, __LINE__, __func__, (fmt), ##arg)

#define LOG_META_FILENAME "log.meta"
#define LOG_DEF_THRESHOLD (1 << 30) // 1GB
#define LOG_DEF_WAIT_MS   200000 // us  

namespace flame {

enum LogLevel {
    // lower
    DEAD = 0,       // dead: crash and exit
    CRITICAL = 1,   // critical: 
    WRONG = 2,      // wrong

    // middle
    ERROR = 3,      // error
    WARN = 4,       // warning
    INFO = 5,       // information

    // upper
    DEBUG = 6,      // debug
    TRACE = 7,      // trace
    PRINT = 8       // print
};

extern const char* LogDict[];

class LogPrinter final {
public:
    explicit LogPrinter(int level) 
    : fp_(stdout), level_(level), imm_flush_(true) {}
    explicit LogPrinter(FILE* fp) 
    : fp_(fp), level_(LogLevel::INFO), imm_flush_(true) {}
    explicit LogPrinter(FILE* fp = stdout, int level = LogLevel::INFO) 
    : fp_(fp), level_(level), imm_flush_(true) {}

    ~LogPrinter();

    void set_file(FILE* fp) { fp_ = fp; }
    FILE* get_file() const { return fp_; }
    void set_level(int level) { level_ = level; }
    int get_level() const { return level_; }
    long int size() const { return ftell(fp_); }
    bool is_imm_flush() const { return imm_flush_; }
    void set_imm_flush(bool v) { imm_flush_ = v; }

    void flush() const { fflush(fp_); }

    template<typename ...Args>
    inline void plog(int level, const char* module, const char* file, int line, const char* func, const char* fmt, const Args &... args) {
        if (level <= level_) {
            char buff[MAX_LOG_LEN];
            int r = snprintf(buff, MAX_LOG_LEN, "[%s][%s] %s(%d) %s(): ", module, LogDict[level], file, line, func);
            r += snprintf(buff + r, MAX_LOG_LEN - r, fmt, args...);
            snprintf(buff + r, MAX_LOG_LEN - r, "\n");
            fprintf(fp_, buff);
            if (imm_flush_) fflush(fp_);
        }
    }

private:
    std::atomic<FILE*> fp_; // primary file pointor
    std::atomic<int> level_;
    std::atomic<bool> imm_flush_; // immediately flush
}; // class LogPrinter

class Logger final {
public:
    explicit Logger(int level = LogLevel::INFO) : printer_(level), threshold_(LOG_DEF_THRESHOLD) {}
    explicit Logger(const std::string& dir, const std::string& prefix, int level = LogLevel::INFO)
    : dir_(dir), prefix_(prefix), printer_(level), threshold_(LOG_DEF_THRESHOLD) {
        switch_log_file();
    }
    ~Logger() {}

    void set_level(int level) { printer_.set_level(level); }
    int get_level() const { return printer_.get_level(); }
    void set_threshold(long int t) { threshold_ = t; }
    long int get_threshold() const { return threshold_; }
    bool is_imm_flush() const { return printer_.is_imm_flush(); }
    void set_imm_flush(bool v) { printer_.set_imm_flush(v); }

    void flush() const { printer_.flush(); }
    bool reopen(const std::string& dir, const std::string& prefix);

    template<typename ...Args>
    inline void plog(int level, const char* module, const char* file, int line, const char* func, const char* fmt, const Args &... args) {
        printer_.plog(level, module, file, line, func, fmt, args...);
        if (level <= printer_.get_level())
            check_and_switch(LOG_DEF_WAIT_MS);
    }

    template<typename ...Args>
    inline void klog(int level, const char* module, const char* file, int line, const char* func, const char* fmt, const Args &... args) {
        printer_.plog(level, module, file, line, func, fmt, args...);
    }

    bool switch_log_file(useconds_t us = 0);
    bool switch_stdout(useconds_t us = 0) { switch_file__(stdout, us); }
    bool switch_stderr(useconds_t us = 0) { switch_file__(stderr, us); }
    bool check_and_switch(useconds_t us = 0);
    
private:
    int read_log_index__();
    bool write_log_index__(int idx);
    void switch_file__(FILE* fp, useconds_t us);

    LogPrinter printer_;
    std::string dir_;
    std::string prefix_;
    long int threshold_;
};

} // namespace flame

#endif // FLAME_INCLUDE_LOG_H