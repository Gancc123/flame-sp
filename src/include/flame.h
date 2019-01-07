#ifndef FLAME_INCLUDE_FLAME_H
#define FLAME_INCLUDE_FLAME_H

#include "common/context.h"
#include "include/meta.h"

#include <memory>
#include <list>

namespace flame {

struct chunk_attr_t {
    uint64_t    chk_id    {0};
    uint64_t    vol_id    {0};
    uint32_t    index     {0};
    uint32_t    stat      {0};
    uint32_t    spolicy   {0};
    uint64_t    primary   {0};
    uint64_t    size      {0};
    uint64_t    csd_id    {0};
    uint64_t    dst_id    {0};
}; // struct chunk_attr_t

class FlameClient {
public:
    virtual ~FlameClient() {}

    //* Gateway Set
    // GW注册： 建立一个GW连接
    virtual int connect(uint64_t gw_id, uint64_t admin_addr) = 0;
    
    // GW注销：关闭一个GW连接
    virtual int disconnect(uint64_t gw_id) = 0;
    
    // 获取Flame集群信息
    virtual int get_cluster_info(cluster_meta_t& res) = 0;
    
    // 自动关闭Flame集群
    virtual int shutdown_cluster() = 0;
    
    // 自动清理Flame集群
    virtual int clean_clustrt() = 0;
    
    // 获取CSD地址信息，需要指定一系列CSD ID
    virtual int pull_csd_addr(std::list<csd_addr_t>& res, const std::list<uint64_t>& csd_id_list) = 0;

    //* Group Set
    // 获取所有VG信息，支持分页（需要提供<offset, limit>，以vg_name字典顺序排序）
    virtual int get_vol_group_list(std::list<volume_group_meta_t>& res, uint32_t offset, uint32_t limit) = 0;
    
    // 创建VG
    virtual int create_vol_group(const std::string& name) = 0;
    
    // 删除VG
    virtual int remove_vol_group(const std::string& name) = 0;
    
    // 重命名VG
    virtual int rename_vol_group(const std::string& old_name, const std::string& new_name) = 0;

    //* Volume Set
    // 获取指定VG内的所有Volume信息
    virtual int get_volume_list(std::list<volume_meta_t>& res, const std::string& name, uint32_t offset, uint32_t limit) = 0;
    
    // 创建Volume
    virtual int create_volume(const std::string& vg_name, const std::string& vol_name, const vol_attr_t& attr) = 0;
    
    // 删除Volume
    virtual int remove_volume(const std::string& vg_name, const std::string& vol_name) = 0;
    
    // 重命名Volume
    virtual int rename_volume(const std::string& vg_name, const std::string& old_vol_name, const std::string& new_vol_name) = 0;
    
    // 获取Volume信息
    virtual int get_volume_info(volume_meta_t& res, const std::string& vg_name, const std::string& vol_name, uint32_t retcode) = 0;
    
    // 更改Volume大小
    virtual int resize_volume(const std::string& vg_name, const std::string& vol_name, uint64_t new_size) = 0;
    
    // 打开Volume：在MGR登记Volume访问信息（没有加载元数据信息）
    virtual int open_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) = 0;
    
    // 关闭Volume：在MGR消除Volume访问信息
    virtual int close_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) = 0;
    
    // 锁定Volume：在MGR登记Volume锁定信息，防止其他GW打开Volume
    virtual int lock_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) = 0;
    
    // 解锁Volume
    virtual int unlock_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) = 0;
    
    // 获取Volume的Chunk信息
    virtual int get_volume_maps(std::list<chunk_attr_t>& res, uint64_t vol_id) = 0;

    //* Chunk Set
    // 获取指定Chunk信息
    virtual int get_chunk_maps(std::list<chunk_attr_t>& res, const std::list<uint64_t>& chk_list) = 0;

protected:
    FlameClient(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class FlameClient

class FlameClientContext {
public:
    FlameClientContext(FlameContext* fct, FlameClient* client) 
    : fct_(fct), client_(client) {}

    ~FlameClientContext() {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }
    
    FlameClient* client() const { return client_.get(); }
    void set_client(FlameClient* c) { client_.reset(c); }

private:
    FlameContext* fct_;
    std::unique_ptr<FlameClient> client_;
}; // class FlameClientContext

} // namespace flame

#endif // FLAME_INCLUDE_FLAME_H