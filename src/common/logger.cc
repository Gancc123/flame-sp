#include "common/logger.h"

#define LOGGER_FILE_SIZE_D  5242880 // (1 << 20) * 5
#define LOGGER_FILE_CNT_D   3
#define LOGGER_QSIZE_D      8192
#define LOGGER_HOUR_D       24
#define LOGGER_MINUTE_D     0

namespace flame {

std::shared_ptr<spd::logger> 
log_open_(const std::string& name, int type, const std::string& path) {
    std::shared_ptr<spd::logger> logger;
    try {
        switch (type) {
        case LoggerType::ROTATING:
            logger = spd::rotating_logger_mt(name, path, LOGGER_FILE_SIZE_D, LOGGER_FILE_CNT_D);
            break;
        case LoggerType::DAILY:
            logger = spd::daily_logger_mt(name, path, LOGGER_HOUR_D, LOGGER_MINUTE_D);
            break;
        case LoggerType::CONSOLE:
            logger = spd::stdout_color_mt(name);
            break;
        case LoggerType::BASIC:
        default:
            logger = spd::basic_logger_mt(name, path);
            break;
        }
    } catch (const spd::spdlog_ex &ex) {
        clog(ex.what());
        return nullptr;
    }
    return logger;
}

Logger::Logger(const std::string& name, int type, const std::string& path)
    : name_(name), logger_(nullptr) {
    logger_ = log_open_(name_, type, path);
}

void Logger::reopen(int type, const std::string& path) {
    close();
    logger_ = log_open_(name_, type, path);
}

} // namespace flame