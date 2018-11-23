#include "config.h"
#include "context.h"

#include <cassert>
#include <fstream>

namespace flame {

static void __load(FlameConfig* fct, std::fstream& fp) {
    assert(fct != nullptr);
}

std::shared_ptr<FlameConfig> FlameConfig::create_config(FlameContext* fct, const std::string &path) {
    std::fstream fp;
    fp.open(path, std::fstream::in);
    if (!fp.is_open()) {
        fct->log()->error("config", "file %s is not existed.", path.c_str());
        return nullptr;
    }
    
    FlameConfig* config = new FlameConfig();
    
    __load(config, fp);

    fp.close();
    return std::shared_ptr<FlameConfig>(config);
}

std::string FlameConfig::get(const std::string& key, const std::string& def_val) {
    if (kvargs_.find(key) != kvargs_.end())
        return kvargs_[key];
    return def_val;
}

void FlameConfig::set(const std::string& key, const std::string& value) {
    kvargs_[key] = value;
}

} // namespace flame