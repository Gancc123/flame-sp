#ifndef FLAME_VOL_MGMT_H
#define FLAME_VOL_MGMT_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "include/objects.h"
#include "common/thread/rw_lock.h"
#include "common/thread/cond.h"
#include "include/flame.h"
#include "include/meta.h"
#include "layout/layout.h"
#include "mgr/chk/chk_mgmt.h"

#include <cstdint>
#include <atomic>
#include <memory>
#include <map>

namespace flame {

class VolManager final {
public:
    VolManager(FlameContext* fct, const std::shared_ptr<MetaStore>& ms, StaticLayoutBase* layout,
    ChkManager* chk_mgmt)
    : fct_(fct), ms_(ms), layout_(layout), chk_mgmt_(chk_mgmt) {}

    int init_vg_map();
    int vg_map_find(const std::string& vg_name, uint64_t& vg_id);

    /**
     * volume group
    */

    int volume_group_list(std::list<volume_group_meta_t>& res_list, const uint32_t& offset, const uint32_t& limit);

    int volume_group_create(const std::string& name);

    int volume_group_remove(const std::string& name);

    int volume_group_rename(const std::string& old_name, const std::string& new_name);

    /**
     * volume
    */

    int volume_list(std::list<volume_meta_t>& res_list, const std::string& vg_name, const uint32_t& offset, const uint32_t& limit);
 
    int volume_create(const std::string& vg_name, volume_meta_t& vol_req);

    int volume_remove(const std::string& vg_name, const std::string& vol_name);

    int volume_rename(const std::string& vg_name, const std::string& vol_old_name, const std::string& vol_new_name);

    int volume_get(volume_meta_t& vol_res, const std::string& vg_name, const std::string& vol_name);

    int volume_resize(const std::string& vg_name, const std::string& vol_name, const uint64_t& new_size);

    int volume_open(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name);

    int volume_close(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name);

    int volume_lock(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name);

    int volume_unlock(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name);

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    StaticLayoutBase* layout_;
    ChkManager* chk_mgmt_;

    std::map<std::string, uint64_t> vg_map_;
    RWLock vg_map_lock_;

}; // class VolManager

} // namespace flame

#endif