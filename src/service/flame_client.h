#ifndef FLAME_SERVICE_FLAME_CLIENT_H
#define FLAME_SERVICE_FLAME_CLIENT_H

#include <memory>
#include <string>
#include <cstdint>


#include "include/flame.h"
#include "common/context.h"
#include "metastore/metastore.h"

#include <grpcpp/grpcpp.h>
#include "proto/flame.grpc.pb.h"

namespace flame {

class FlameClientImpl final : public FlameClient {
public:
    FlameClientImpl(FlameContext* fct, std::shared_ptr<grpc::Channel> channel)
    : FlameClient(fct), stub_(FlameService::NewStub(channel)) {}

    //* Gateway Set
    // GW注册： 建立一个GW连接
    int connect(uint64_t gw_id, uint64_t admin_addr) override;

    // GW注销：关闭一个GW连接
    int disconnect(uint64_t gw_id) override;

    // 获取Flame集群信息
    int get_cluster_info(cluster_meta_t& res) override;

    // 自动关闭Flame集群
    int shutdown_cluster() override;

    // 自动清理Flame集群
    int clean_clustrt() override;

    // 获取CSD地址信息，需要指定一系列CSD ID
    int pull_csd_addr(const std::list<uint64_t>& csd_id_list, std::list<csd_addr_attr_t>& res) override;

    //* Group Set
    // 获取所有VG信息，支持分页（需要提供<offset, limit>，以vg_name字典顺序排序）
    int get_vol_group_list(uint32_t offset, uint32_t limit, std::list<volume_group_meta_t>& res) override;

    // 创建VG
    int create_vol_group(const std::string& name) override;

    // 删除VG
    int remove_vol_group(const std::string& name) override;

    // 重命名VG
    int rename_vol_group(const std::string& old_name, const std::string& new_name) override;

    //* Volume Set
    // 获取指定VG内的所有Volume信息
    int get_volume_list(const std::string& name, uint32_t offset, uint32_t limit, std::list<volume_meta_t>& res) override;

    // 创建Volume
    int create_volume(volume_attr_t& vol_attr) override;

    // 删除Volume
    int remove_volume(const std::string& vg_name, const std::string& vol_name) override;

    // 重命名Volume
    int rename_volume(const std::string& vg_name, const std::string& old_vol_name, const std::string& new_vol_name) override;

    // 获取Volume信息
    int get_volume_info(const std::string& vg_name, const std::string& vol_name, uint32_t retcode, volume_meta_t& res) override;

    // 更改Volume大小
    int resize_volume(const std::string& vg_name, const std::string& vol_name, uint64_t new_size) override;

    // 打开Volume：在MGR登记Volume访问信息（没有加载元数据信息）
    int open_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) override;

    // 关闭Volume：在MGR消除Volume访问信息
    int close_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) override;

    // 锁定Volume：在MGR登记Volume锁定信息，防止其他GW打开Volume
    int lock_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) override;

    // 解锁Volume
    int unlock_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) override;

    // 获取Volume的Chunk信息
    int get_volume_maps(uint64_t vol_id, std::list<chunk_attr_t>& res) override;

    //* Chunk Set
    // 获取指定Chunk信息
    int get_chunk_maps(const std::list<uint64_t>& chk_list, std::list<chunk_attr_t>& res) override;

private:
    std::unique_ptr<FlameService::Stub> stub_;

    int return_error__(const grpc::Status& stat);
}; // class FlameClientImpl

} // namespace flame


#endif // FLAME_SERVICE_FLAME_CLIENT_H