#ifndef FLAME_CHUNKSTORE_SIM_H
#define FLAME_CHUNKSTORE_SIM_H

#include "common/context.h"
#include "chunkstore/chunkstore.h"

#include <cstdint>
#include <string>
#include <map>
#include <atomic>
#include <fstream>
#include <vector>

#define SIMSTORE_BLOCK_SIZE (1ULL << 22)    // 4MB

#define SIMSTORE_SEP_L1 ';'
#define SIMSTORE_SEP_L2 ','
#define SIMSTORE_SEP_L3 '/'
#define SIMSTORE_SEP_KW '='

namespace flame {

struct simstore_counter_t {
    int64_t rd {0};
    int64_t wr {0};
};

struct simstore_block_t {
    uint64_t ctime  {0};
    simstore_counter_t cnt;
};

struct simstore_chunk_t {
    chunk_info_t info;
    std::map<std::string, std::string> xattr;
    std::vector<simstore_block_t> blocks;

    void init_blocks__();
}; 

class SimStore final : public ChunkStore {
public:
    static SimStore* create_simstore(FlameContext* fct, const std::string& url);
    static SimStore* create_simstore(FlameContext* fct, uint64_t size, const std::string& bk_file = "");

    virtual ~SimStore() {}

    virtual int set_info(const cs_info_t& info) override { info_ = info; return 0; }
    virtual int get_info(cs_info_t& info) const override;
    virtual std::string get_driver_name() const override;
    virtual std::string get_config_info() const override;
    virtual std::string get_runtime_info() const override;
    virtual int get_io_mode() const override;

    virtual int dev_check() override;
    virtual int dev_format() override;
    virtual int dev_mount() override;
    virtual int dev_unmount() override;
    virtual bool is_mounted() override;

    virtual int chunk_create(uint64_t chk_id, const chunk_create_opts_t& opts) override;
    virtual int chunk_remove(uint64_t chk_id) override;
    virtual bool chunk_exist(uint64_t chk_id) override;
    virtual std::shared_ptr<Chunk> chunk_open(uint64_t chk_id) override;

    friend class SimChunk;

private:
    SimStore(FlameContext* fct, uint64_t size, const std::string& bk_file) 
    : ChunkStore(fct), size_(size), bk_file_(bk_file), formated_(false), mounted_(false) {
        info_.size = size;    
    }

    bool formated_;
    bool mounted_;
    uint64_t size_;
    std::string bk_file_;
    cs_info_t info_;
    std::map<uint64_t, simstore_chunk_t> chk_map_;

    int info_init__();    
    int backup_load__();
    int backup_store__();

    // 基于状态转换读取backup_file
    int ld_upper__(std::fstream& fin);          // error code
    int ld_annotation__(std::fstream& fin);     // -
    int ld_sub_name__(std::fstream& fin);       // 1
    int ld_sub_kv__(std::fstream& fin);         // 2
    int ld_sub_table__(std::fstream& fin);      // 3
    int ld_sub_table_header__(std::fstream& fin, std::vector<std::string>& header);
    int ld_sub_table_row__(std::fstream& fin, std::vector<std::string>& header);
    
    int st_info__(const std::string& key, const std::string& value);
    int st_chunk__(simstore_chunk_t& chk, const std::string& key, const std::string& value);
    int st_chunk_xattr__(simstore_chunk_t& chk, const std::string& value);
    int st_chunk_blocks__(simstore_chunk_t& chk, const std::string& value);
}; // class SimStore

class SimChunk final : public Chunk {
public:
    virtual ~SimChunk() {}

    /**
     * 基本信息部分
     */
    virtual int get_info(chunk_info_t& info) const override;
    virtual uint64_t size() const override;
    virtual uint64_t used() const override;
    virtual uint32_t stat() const override;
    virtual uint64_t vol_id() const override;
    virtual uint32_t vol_index() const override;
    virtual uint32_t spolicy() const override;
    virtual bool is_preallocated() const override;

    /**
     * 操作部分
     */
    virtual int read_sync(void* buff, uint64_t off, uint64_t len) override;
    virtual int write_sync(void* buff, uint64_t off, uint64_t len) override;
    virtual int get_xattr(const std::string& name, std::string& value) override;
    virtual int set_xattr(const std::string& name, const std::string& value) override;
    virtual int read_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) override;
    virtual int write_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) override;

    friend class SimStore;
private:
    SimChunk(FlameContext* fct, SimStore* parent, simstore_chunk_t* chk) 
    : Chunk(fct), parent_(parent), chk_(chk) {}

    void blk_range__(uint32_t& begin, uint32_t& end, uint64_t off, uint64_t len);
    void rd_count__(uint64_t off, uint64_t len);
    void wr_count__(uint64_t off, uint64_t len);

    SimStore* parent_;
    simstore_chunk_t* chk_;
}; // class Chunk

} // namespace flame

#endif // FLAME_CHUNKSTORE_SIM_H