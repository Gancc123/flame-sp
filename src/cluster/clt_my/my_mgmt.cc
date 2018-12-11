#include "cluster/clt_my/my_mgmt.h"

using namespace flame;

void CheckStatWork::entry() {
    SpinLocker(mgmt_->stmap_lock_);
    for (auto it = mgmt_->stmap_.begin(); it != mgmt_->stmap_.end(); ++it) {
        if (it->second.latime + mgmt_->durable_time_ < utime_t::now()) {
            it->second.stat = 0;

            cb_(mgmt_->fct_, it->first, 0);
        }
    }
}