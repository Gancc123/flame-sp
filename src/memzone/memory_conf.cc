#include "msg/internal/util.h"
#include "memzone/memory_conf.h"

#include <cstring>
#include <string>
#include <iostream>
#include <regex>
#include <utility>

namespace flame {
namespace memory {

bool MemoryConfig::get_bool(const std::string &v) {
    const char *true_values[] = {"yes", "enable", "on", "true", nullptr};
    auto lv = flame::msg::str2lower(v);
    const char **it = true_values;
    while(*it){
        if(lv.compare(*it) == 0){
            return true;
        }
        ++it;
    }
    return false;
}

int MemoryConfig::set_rdma_enable(const std::string &v) {
    rdma_enable = false;
    if(v.empty()) {
        return 1;
    }
    rdma_enable = MemoryConfig::get_bool(v);
    return 0;
}

int MemoryConfig::set_rdma_device_name(const std::string &v) {
    if(v.empty()) {
        return 1;
    }
    rdma_device_name = v;
    return 0;
}

int MemoryConfig::set_rdma_buffer_num(const std::string &v){
    rdma_buffer_num = 0;
    if(v.empty()){
        return 0;
    }

    int nbuf = std::stoi(v, nullptr, 0);

    if(nbuf >= 0){
        rdma_buffer_num = nbuf;
        return 0;
    }

    // error
    return 1;
}

int MemoryConfig::set_rdma_buffer_size(const std::string &v){
    if(v.empty()){
        return 1;
    }
    uint64_t result = flame::msg::size_str_to_uint64(v);
    if(result < (1ULL << 32)){
        rdma_buffer_size = result;
        return 0;
    }
    return 1;
}

int MemoryConfig::set_rdma_enable_hugepage(const std::string &v){
    rdma_enable_hugepage = false;
    if(v.empty()){
        return 1;
    }
    rdma_enable_hugepage = MemoryConfig::get_bool(v);
    return 0;
}

int MemoryConfig::set_rdma_mem_min_level(const std::string &v){
    if(v.empty()){
        return 1;
    }

    int min_level = std::stoi(v, nullptr, 0);
    if(min_level >= 0 && min_level < (1U << 8)){
        rdma_mem_min_level = min_level;
        return 0;
    }

    return 1;
}

int MemoryConfig::set_rdma_mem_max_level(const std::string &v){
    if(v.empty()){
        return 1;
    }

    int max_level = std::stoi(v, nullptr, 0);
    if(max_level >= 0 && max_level < (1U << 8)){
        rdma_mem_max_level = max_level;
        return 0;
    }

    return 1;
}

int MemoryConfig::load(){
    int res = 0;
    auto cfg = fct->config();

    res = set_rdma_enable(cfg->get("rdma_enable", FLAME_MEMORY_RDMA_ENABLE_D));
    if (res) {
        perr_arg("rdma_enable");
        return 1;
    }

    if (rdma_enable) {
        res = set_rdma_device_name(cfg->get("rdma_device_name", 
                                                FLAME_MEMORY_RDMA_DEVICE_NAME_D));
        if (res) {
            perr_arg("rdma_device_name");
            return 1;
        }

        res = set_rdma_buffer_num(cfg->get("rdma_buffer_num", 
                                            FLAME_MEMORY_RDMA_BUFFER_NUM_D));
        if (res) {
            perr_arg("rdma_buffer_num");
            return 1;
        }

        res = set_rdma_buffer_size(cfg->get("rdma_buffer_size", 
                                            FLAME_MEMORY_RDMA_BUFFER_SIZE_D));
        if (res) {
            perr_arg("rdma_buffer_size");
            return 1;
        }

        res = set_rdma_enable_hugepage(cfg->get("rdma_enable_hugepage", 
                                                FLAME_MEMORY_RDMA_ENABLE_HUGEPAGE_D));
        if (res) {
            perr_arg("rdma_enable_hugepage");
            return 1;
        }

        res = set_rdma_mem_min_level(cfg->get("rdma_mem_min_level", 
                                                FLAME_MEMORY_RDMA_MEM_MIN_LEVEL_D));
        if (res) {
            perr_arg("rdma_mem_min_level");
            return 1;
        }
        
        res = set_rdma_mem_max_level(cfg->get("rdma_mem_max_level", 
                                                FLAME_MEMORY_RDMA_MEM_MAX_LEVEL_D));
        if (res) {
            perr_arg("rdma_mem_max_level");
            return 1;
        }
    }

    return 0;
}

}   //namespace memory
}   //namesapce flame