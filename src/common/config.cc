#include "config.h"
#include "context.h"

#include <cstring>
#include <cassert>
#include <fstream>
#include <exception>

namespace flame {

static inline char* __trim(char *start, char *end) {
    while (start <= end && std::strchr(" \t\n\r", *start))
        start++;
    return start;
}

static inline char* __trim_r(char *start, char *end) {
    while (start <= end && std::strchr(" \t\n\r", *end))
        end--;
    return end;
}

static std::string __gen_string(char *start, char *end) {
    std::string res;
    ssize_t len;
    char *ps, *pe;
    ps = __trim(start, end);
    pe = __trim_r(ps, end);
    len = pe - ps + 1;
    if (len > 0) 
        res.append(ps, len);
    return res;        
}

static bool __load(FlameContext* fct, FlameConfig* cfg, std::fstream& fp) {
    assert(fct != nullptr);
    char buff[256], *ps, *pe, *pm;
    size_t len, len_key, len_value;
    int line = 0;
    while (!fp.eof() && fp.getline(buff, 255)) {
        line++;
        len = strlen(buff);
        if (len == 0)
            continue;
        ps = __trim(buff, buff + len - 1);
        if (ps >= buff + len || *ps == '\0' || *ps == '#')
            continue;

        pm = std::strchr(ps, '=');
        if (pm == NULL) {
            fct->log()->lerror("config", "invalid config item in line %d, it must be key=value", line);
            return false;
        }
        
        std::string key = __gen_string(ps, pm - 1);
        if (key.empty()) {
            fct->log()->lerror("config", "invalid config item in line %d, key can't be empty", line);
            return false;
        }
        
        std::string value = __gen_string(pm + 1, buff + len - 1);
        if (value.empty()) {
            fct->log()->lerror("config", "invalid config item in line %d, value can't be empty", line);
            return false;
        }

        if (cfg->has_key(key)) {
            fct->log()->lwarn("config", "duplicate definition with key('%s'), in line %d", key.c_str(), line);
        }
        if(strcmp(key.c_str(),"cluster_name") == 0)
            fct->set_cluster_name(value);
        else if(strcmp(key.c_str(),"csd_name") == 0) //这里应该不仅限于csd_name
            fct->set_node_name(value);
        cfg->set(key, value);
    }
    return true;
}

FlameConfig* FlameConfig::create_config(FlameContext* fct, const std::string &path) {
    std::fstream fp;
    fp.open(path, std::fstream::in);
    if (!fp.is_open()) {
        fct->log()->lerror("config", "file %s is not existed.", path.c_str());
        return nullptr;
    }
    
    FlameConfig* config = new FlameConfig();
    
    bool r = __load(fct, config, fp);
    fp.close();

    if (r)
        return config;
    else {
        delete config;
        return nullptr;
    }
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