#ifndef FLAME_INCLUDE_CONTEXT_H
#define FLAME_INCLUDE_CONTEXT_H

#include "log.h"
#include "config.h"

namespace flame {
    
class FlameContext {
public:
    static FlameContext* get_context() {
        if (g_context == nullptr) {
            g_context = new FlameContext();
        }
        return FlameContext::g_context; 
    }
    static FlameContext* g_context;

    FlameContext() : log_(nullptr), config_(nullptr) {}
    
    Logger* log() const { return log_; }
    void set_log(Logger* log);

    FlameConfig* config() const { return config_; }
    void set_config(FlameConfig* config);

private:
    FlameContext(const FlameContext&) = delete;
    FlameContext& operator = (const FlameContext&) = delete;
    FlameContext(FlameContext&&) = delete;
    FlameContext& operator = (FlameContext&&) = delete;

    Logger* log_;
    FlameConfig* config_;
}; // class FlameContext

} // namespace flame


#endif // FLAME_INCLUDE_CONTEXT_H