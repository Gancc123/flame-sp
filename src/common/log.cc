#include "log.h"

#include "util/clog.h"
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>

namespace flame {

const char* LogDict[] = {
    // lower
    "DEAD",       // dead: crash and exit
    "CRITICAL",   // critical: 
    "WRONG",      // wrong

    // middle
    "ERROR",      // error
    "WARN",       // warning
    "INFO",       // information

    // upper
    "DEBUG",      // debug
    "TRACE",      // trace
    "PRINT"       // print
};

LogPrinter::~LogPrinter() {
    if (fp_ != nullptr && fp_ != stdout && fp_ != stderr) {
        fflush(fp_);
        fclose(fp_);
    }
}

bool Logger::set_level_with_name(const std::string& level_name) {
    for (int i = 0; i < 9; i++) {
        if (level_name == LogDict[i]) {
            set_level(i);
            return true;
        }
    }
    return false;
}

bool Logger::reopen(const std::string& dir, const std::string& prefix) {
    dir_ = dir;
    prefix_ = prefix;

    int res = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(res){
        if(errno != EEXIST){
            lerror("logger", "create log dir(%s) failed", dir.c_str());
            return false;
        }
    }

    return switch_log_file();
}

int Logger::read_log_index__() {
    std::string path = string_concat({dir_, "/", prefix_, LOG_META_FILENAME});
    FILE* fp = fopen(path.c_str(), "r");
    if (fp == nullptr)
        return 0;
    int idx;
    fscanf(fp, "%d", &idx);
    fclose(fp);
    return idx + 1;
}

bool Logger::write_log_index__(int idx) {
    std::string path = string_concat({dir_, "/", prefix_, LOG_META_FILENAME});
    FILE* fp = fopen(path.c_str(), "w");
    if (fp == nullptr) {
        lerror("logger", "open log file(%s) faild", path.c_str());
        return false;
    }
    fprintf(fp, "%d", idx);
    fclose(fp);
    return true;
}

bool Logger::switch_log_file(useconds_t us) {
    int idx = read_log_index__();

    std::string path = string_concat({dir_, "/", prefix_, ".", convert2string(idx), ".log"});
    FILE* fp = fopen(path.c_str(), "w");
    if (fp == nullptr) {
        lerror("logger",  "open log file(%s) faild", path.c_str());
        return false;
    }
    switch_file__(fp, us);

    return write_log_index__(idx);
}

bool Logger::check_and_switch(useconds_t us) {
    if (printer_.size() >= threshold_) {
        switch_log_file(us);
        return true;
    }
    return false;
}

void Logger::switch_file__(FILE* fp, useconds_t us) {
    FILE* old = printer_.get_file();
    printer_.set_file(fp);
    if (old != stdout && old != stderr) {
        if (us != 0)
            usleep(us);
        fclose(old);
    }
}

} // namespace flame