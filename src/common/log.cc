#include "log.h"

#include <cstdio>
#include <unistd.h>

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

static int read_log_index(const std::string& dir) {
    std::string path = string_concat({dir, "/", LOG_META_FILENAME});
    FILE* fp = fopen(path.c_str(), "r");
    if (fp == nullptr)
        return 0;
    int idx;
    fscanf(fp, "%d", &idx);
    fclose(fp);
    return idx + 1;
}

static bool write_log_index(const std::string& dir, int idx) {
    std::string path = string_concat({dir, "/", LOG_META_FILENAME});
    FILE* fp = fopen(path.c_str(), "w");
    if (fp == nullptr)
        return false;
    fprintf(fp, "%d", idx);
    fclose(fp);
    return true;
}

bool Logger::switch_log_file(useconds_t us) {
    int idx = read_log_index(dir_);

    std::string path = string_concat({prefix_, ".", convert2string(idx), ".log"});
    FILE* fp = fopen(path.c_str(), "w");
    if (fp == nullptr)
        return false;
    switch_file_(fp, us);

    return write_log_index(dir_, idx);
}

bool Logger::check_and_switch(useconds_t us) {
    if (printer_.size() >= threshold_) {
        switch_log_file(us);
        return true;
    }
    return false;
}

void Logger::switch_file_(FILE* fp, useconds_t us) {
    FILE* old = printer_.get_file();
    printer_.set_file(fp);
    if (old != stdout || old != stderr) {
        if (us != 0)
            usleep(us);
        fclose(old);
    }
}

} // namespace flame