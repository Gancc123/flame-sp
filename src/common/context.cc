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


} // namespace flame