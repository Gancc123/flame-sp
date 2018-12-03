#include "context.h"

namespace flame {

FlameContext* FlameContext::g_context = nullptr;

void FlameContext::set_log(Logger* log) {
    if (log_ != nullptr)
        delete log_;
    log_ = log;
}

void FlameContext::set_config(FlameConfig* config) {
    if (config_ != nullptr)
        delete config_;
    config_ = config;
}

bool FlameContext::init_config(const std::string& path) {
    FlameConfig* config = FlameConfig::create_config(this, path);
    if (config == nullptr) {
        log()->error("FlameContext", "read config file (%s) faild.", path.c_str());
        return false;
    }
    set_config(config);
    return true;
}

bool FlameContext::init_log(const std::string& dir, const std::string& level, const std::string& prefix) {
    std::string log_dir;
    if (dir.empty()) {
        if (config()->has_key(CFG_LOG_DIR)) {
            log_dir = config()->get(CFG_LOG_DIR, ".");
        } else {
            log()->error("FlameContext", "missing config item '%s'", CFG_LOG_DIR);
            return false;
        }
    } else {
        log_dir = dir;
    }

    if (!log()->set_level_with_name(level)) {
        log()->error("FlameContext", "set log level faild");
        return false;
    }
    
    if (!log()->reopen(log_dir, prefix)) {
        log()->error("FlameContext", "reopen logger to file faild");
        return false;
    }
    return true;
}

} // namespace flame