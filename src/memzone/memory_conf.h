#ifndef FLAME_MEMORY_INTERNAL_CONFIG_H
#define FLAME_MEMORY_INTERNAL_CONFIG_H

#include "util/clog.h"
#include "common/context.h"

#include <string>
#include <vector>
#include <tuple>
#include <cassert>

#define FLAME_MEMORY_RDMA_ENABLE_D           "false"
#define FLAME_MEMORY_RDMA_DEVICE_NAME_D      ""
#define FLAME_MEMORY_RDMA_BUFFER_NUM_D       ""
#define FLAME_MEMORY_RDMA_BUFFER_SIZE_D      "4080"
#define FLAME_MEMORY_RDMA_ENABLE_HUGEPAGE_D  "true"
#define FLAME_MEMORY_RDMA_MEM_MIN_LEVEL_D    "12"
#define FLAME_MEMORY_RDMA_MEM_MAX_LEVEL_D    "28"

namespace flame {
namespace memory {

class MemoryConfig {
private:
    FlameContext *fct;

    void perr_arg(const std::string &v) {
        clog(v);
    }

public:
    static MemoryConfig *load_config(FlameContext *c) {
        auto config = new MemoryConfig(c);
        assert(config);

        if(config->load()) {
            delete config;
            return nullptr;
        }

        return config;
    }

    explicit MemoryConfig(FlameContext *c): fct(c) {

    }

    static bool get_bool(const std::string&v);

    int load();
    /**
     * RDMA enable
     * @cfg: rdma_enable
     */
    bool rdma_enable;
    int set_rdma_enable(const std::string &v);

    /**
     * RDMA device name
     * @cfg: rdma_device_name
     */
    std::string rdma_device_name;
    int set_rdma_device_name(const std::string &v);

    /**
     * RDMA buffer number
     * @cfg: rdma_buffer_num
     * 0 means no limit.
     */
    int  rdma_buffer_num;
    int set_rdma_buffer_num(const std::string &v);

    /**
     * RDMA buffer size
     * @cfg: rdma_buffer_size
     */
    uint32_t rdma_buffer_size;
    int set_rdma_buffer_size(const std::string &v);

    /**
     * RDMA enable hugepage
     * @cfg: rdma_enable_hugepage
     */
    uint32_t rdma_enable_hugepage;
    int set_rdma_enable_hugepage(const std::string &v);

    /**
     * RDMA RdmaBuffer min size (2^min_level)
     * @cfg: rdma_mem_min_level
     */
    uint8_t  rdma_mem_min_level;
    int set_rdma_mem_min_level(const std::string &v);

    /**
     * RDMA RdmaBuffer max size (2^max_level).
     * One BuddyAllocator will have 2^max_level bytes.
     * @cfg: rdma_mem_max_level
     */
    uint8_t  rdma_mem_max_level;
    int set_rdma_mem_max_level(const std::string &v);
};

}   //end namespace memory
}   //end namespace flame

#endif