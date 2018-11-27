#ifndef FLAME_SERVICE_FLAME_CLIENT_H
#define FLAME_SERVICE_FLAME_CLIENT_H

#include <memory>
#include <string>
#include <cstdint>


#include "common/context.h"
#include "proto/flame.grpc.pb.h"
#include "metastore/metastore.h"

namespace flame {

struct csd_addr_attr_t{
    uint64_t    csd_id    {0};
    uint64_t    io_addr   {0};
    uint32_t    stat      {0};
}; //struct csd_addr_attr_t

struct volume_attr_t{
    std::string vg_name   { };
    std::string vol_name  { };
    uint64_t    chk_sz    {0};
    uint64_t    size      {0};
    uint32_t    flags     {0};
    uint32_t    spolicy   {0};
}; //struct volume_attr_t

struct chunk_attr_t{
    uint64_t    chk_id    {0};
    uint64_t    vol_id    {0};
    uint32_t    index     {0};
    uint32_t    stat      {0};
    uint32_t    spolicy   {0};
    uint64_t    primary   {0};
    uint64_t    size      {0};
    uint64_t    csd_id    {0};
    uint64_t    dst_id    {0};
}; //struct chunk_attr_t


class FlameClient {
public:
    FlameClient(FlameContext* fct, std::shared_ptr<grpc::Channel> channel)
    : fct_(fct), stub_(FlameService::NewStub(channel)) {}

    //* Gateway Set
    // GW注册： 建立一个GW连接
    int connect(uint64_t gw_id, uint64_t admin_addr);
    // GW注销：关闭一个GW连接
    int disconnect(uint64_t gw_id);
    // 获取Flame集群信息
    int get_cluster_info(cluster_meta_t& res);
    // 自动关闭Flame集群
    int shutdown_cluster();
    // 自动清理Flame集群
    int clean_clustrt();
    // 获取CSD地址信息，需要指定一系列CSD ID
    int pull_csd_addr(const list<uint64_t>& csd_id_list, list<csd_addr_attr_t>& res);

    //* Group Set
    // 获取所有VG信息，支持分页（需要提供<offset, limit>，以vg_name字典顺序排序）
    int get_vol_group_list(uint32_t offset, uint32_t limit, list<volume_group_meta_t>& res);
    // 创建VG
    int create_vol_group(std::string name);
    // 删除VG
    int remove_vol_group(std::string name);
    // 重命名VG
    int rename_vol_group(std::string old_name, std::string new_name);

    //* Volume Set
    // 获取指定VG内的所有Volume信息
    int get_volume_list(std::string name, uint32_t offset, uint32_t limit, list<volume_meta_t>& res);
    // 创建Volume
    int create_volume(volume_attr_t& vol_attr);
    // 删除Volume
    int remove_volume(std::string vg_name, std::string vol_name);
    // 重命名Volume
    int rename_volume(std::string vg_name, std::string old_vol_name, std::string new_vol_name);
    // 获取Volume信息
    int get_volume_info(std::string vg_name, std::string vol_name, uint32_t retcode, volume_meta_t& res);
    // 更改Volume大小
    int resize_volume(std::string vg_name, std::string vol_name, uint64_t new_size);
    // 打开Volume：在MGR登记Volume访问信息（没有加载元数据信息）
    int open_volume(uint64_t gw_id, std::string vg_name, std::string vol_name);
    // 关闭Volume：在MGR消除Volume访问信息
    int close_volume(uint64_t gw_id, std::string vg_name, std::string vol_name);
    // 锁定Volume：在MGR登记Volume锁定信息，防止其他GW打开Volume
    int lock_volume(uint64_t gw_id, std::string vg_name, std::string vol_name);
    // 解锁Volume
    int unlock_volume(uint64_t gw_id, std::string vg_name, std::string vol_name);
    // 获取Volume的Chunk信息
    int get_volume_maps(uint64_t vol_id, list<chunk_attr_t>& res);

    //* Chunk Set
    // 获取指定Chunk信息
    int get_chunk_maps(list<uint64_t>& chk_list, list<chunk_attr_t>& res);

private:
    FlameContext* fct_;
    std::unique_ptr<FlameService::Stub> stub_;
}; // class FlameClient

} // namespace flame


#endif // FLAME_SERVICE_FLAME_CLIENT_H