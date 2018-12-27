#include "cluster/clt_my/my_mgmt.h"

#include "cluster/log_cluster.h"

namespace flame {

void CheckStatWork::entry() {
    mgmt_->stat_check();
}

int MyClusterMgmt::update_stat(uint64_t node_id, uint32_t stat) {
    SpinLocker locker(stmap_lock_);
    int r;
    if (stmap_update__(node_id, stat)) {
        r = csdm_->csd_stat_update(node_id, stat);
        if (r != RC_SUCCESS) {
            fct_->log()->lerror("csd_stat_update faild");
            return r;
        }
    }
    return RC_SUCCESS;
}

void MyClusterMgmt::stat_check() {
    SpinLocker locker(stmap_lock_);
    utime_t tnow = utime_t::now();
    for (auto it = stmap_.begin(); it != stmap_.end(); ++it) {
        if (it->second.stat == CSD_STAT_ACTIVE
            || it->second.stat == CSD_STAT_PAUSE
            || tnow - it->second.latime > hb_check_
        ) {
            csdm_->csd_stat_update(it->first, CSD_STAT_DOWN);
        } 
    }
}

bool MyClusterMgmt::stmap_update__(uint64_t node_id, uint32_t stat) {
    bool res = false;
    node_stat_item_t& item = stmap_[node_id];
    item.latime = utime_t::now();
    if (item.stat != stat) {
        item.stat = stat;
        return true;
    } else
        return false;
}

} // namespace flame