#ifndef FLAME_CHK_MGMT_H
#define FLAME_CHK_MGMT_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "include/objects.h"
#include "common/thread/rw_lock.h"
#include "include/internal.h"
#include "include/meta.h"
#include "mgr/csdm/csd_mgmt.h"

#include <cstdint>
#include <atomic>
#include <memory>
#include <map>
#include <set>

namespace flame {

class ChkManager final {
public:
    ChkManager(FlameContext* fct, const std::shared_ptr<MetaStore>& ms, CsdManager* csd_mgmt)
    : fct_(fct), ms_(ms), csd_mgmt_(csd_mgmt) {}

    // 以csd为单位发送创建一组chunk的请求
    int chunk_bulk_create(std::map<uint64_t, std::list<uint64_t>>& chk_list, volume_meta_t& vol_req);

    // 以csd为单位发送删除一组chunk的请求
    int chunk_remove_by_volid(const uint64_t& vol_id);

    // 获取Volume的Chunk信息
    int chunk_get_maps(std::list<chunk_meta_t>& chk_list, const uint64_t& vol_id);

    // 获取指定Chunk信息
    int chunk_get_set(std::list<chunk_meta_t>& res_list, const std::list<uint64_t>& chk_ids);

    // 获取关联chunk的信息
    int chunk_get_related(std::list<chunk_meta_t>& res_list, const uint64_t& chk_id);
    int chunk_get_related(std::list<chunk_meta_t>& res_list, const uint64_t& vol_id, const uint16_t& index);

    // csd推送chunk的状态
    int chunk_push_status(const std::list<chk_push_attr_t>& chk_list);

    // 更新chunk的健康信息
    int chunk_push_health(const std::list<chk_hlt_attr_t>& chk_hlt_list);

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    CsdManager* csd_mgmt_;

}; // class ChkManager

} // namespace flame

#endif // FLAME_CHK_MGMT_H