#ifndef FLAME_INCLUDE_CONFIG_H
#define FLAME_INCLUDE_CONFIG_H

#define CFG_CLUSTER_NAME "cluster_name"
#define CFG_CLUSTER_MGRS "cluster_mgrs"
#define CFG_LOG_DIR "log_dir"

#include <exception>
#include <string>
#include <memory>
#include <map>

namespace flame {

class FlameContext;

class FlameConfig final {
public:
    static FlameConfig* create_config(FlameContext* fct, const std::string &path);
    
    FlameConfig() {}
    ~FlameConfig() {}

    bool has_key(const std::string& key) { return kvargs_.find(key) != kvargs_.end(); }
    // Return Value iff it existed, otherwise Return def_val
    std::string get(const std::string& key, const std::string& def_val = "");
    void set(const std::string& key, const std::string& value);

    FlameConfig(const FlameConfig&) = default;
    FlameConfig(FlameConfig&&) = default;
    FlameConfig& operator = (const FlameConfig&) = default;
    FlameConfig& operator = (FlameConfig&&) = default;

private:
    std::map<std::string, std::string> kvargs_;
}; // class FlameConfig

} // namespace flame

#endif // FLAME_INCLUDE_CONFIG_H