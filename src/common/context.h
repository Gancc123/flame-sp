#ifndef FLAME_COMMON_CONTEXT_H
#define FLAME_COMMON_CONTEXT_H

#include "common/log.h"
#include "common/config.h"
#include "msg/msg_module.h"

#include <string>

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

    FlameContext() : log_(new Logger(LogLevel::PRINT)), config_(nullptr) {}
    
    Logger* log() const { return log_; }
    void set_log(Logger* log);
    bool init_config(const std::string& path);
    bool init_log(const std::string& dir, const std::string& level, const std::string& prefix);

    FlameConfig* config() const { return config_; }
    void set_config(FlameConfig* config);

    const std::string& cluster_name() const { return cluster_name_; }
    void set_cluster_name(const std::string& name) { cluster_name_ = name; }

    const std::string& node_name() const { return node_name_; }
    void set_node_name(const std::string& name) { node_name_ = name; } 

    MsgModule* msg() { return msg_module_; }
    void set_msg(MsgModule *m) { msg_module_ = m; }

private:
    FlameContext(const FlameContext&) = delete;
    FlameContext& operator = (const FlameContext&) = delete;
    FlameContext(FlameContext&&) = delete;
    FlameContext& operator = (FlameContext&&) = delete;

    Logger* log_;
    FlameConfig* config_;
    std::string cluster_name_;
    std::string node_name_;
    MsgModule *msg_module_;
}; // class FlameContext

} // namespace flame


#endif // FLAME_COMMON_CONTEXT_H