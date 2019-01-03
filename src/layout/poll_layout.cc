#include "poll_layout.h"

namespace flame  {
namespace layout {

void PollWork::entry() {
    playout_->set_csd_info();
}

void PollLayout::init() {
    tw_->push_cycle(std::shared_ptr<WorkEntry>(new PollWork(this)), cycle_);
}

void PollLayout::set_csd_info() {
    csdm_->read_lock();
    WriteLocker locker(csd_info_lock_);
    csd_info_.clear();

    for (auto it = csdm_->csd_hdl_begin(); it != csdm_->csd_hdl_end(); ++it) {
        CsdObject* obj = it->second->read_and_lock();

        csd_info_[obj->get_csd_id()] = obj->get_size() - obj->get_alloced();

        it->second->unlock();
    }

    csdm_->unlock();

    it_ = csd_info_.begin();
}

int PollLayout::get_next_csd(uint64_t& csd_id, const uint64_t& chk_sz) {
    WriteLocker lock(csd_info_lock_);
    int len = csd_info_.size();
    int cnt = 0;
    while(cnt < len) {
        if (it_ == csd_info_.end()) it_ = csd_info_.begin();
        if (csd_info_[it_->second] >= chk_sz && csdm_->find(it_->first)->is_active()) {
            csd_id = it_->first;
            csd_info_[csd_id] -= chk_sz;
            ++it_;
            return RC_SUCCESS;
        } else {
            ++it_;
        }
        ++cnt;
    }
    return RC_FAILD;
}

int PollLayout::select(std::list<uint64_t>& csd_ids, int num, uint64_t chk_sz) {
    while (num--) {
        uint64_t csd_id;
        if (get_next_csd(csd_id, chk_sz) == RC_SUCCESS) {
            csd_ids.push_back(csd_id);
        } else {
            return RC_FAILD;
        }
        
    }
    return RC_SUCCESS;
}

int PollLayout::select_bulk(std::list<uint64_t>& csd_ids, int grp, int cgn, uint64_t chk_sz) {
    while (grp--) {
        while (cgn--) {
            uint64_t csd_id;
            if (get_next_csd(csd_id, chk_sz) == RC_SUCCESS) {
                csd_ids.push_back(csd_id);
            } else {
                return RC_FAILD;
            }
        }
    }

    return RC_SUCCESS;
}

}
}