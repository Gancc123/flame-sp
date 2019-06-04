#include "poll_layout.h"
#include "common/context.h"
#include "unistd.h"
namespace flame  {
namespace layout {

void PollLayout::init() {
    csdm_->read_lock();
    WriteLocker locker(csd_info_lock_);

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
    bool flag = false;
    FlameContext* fct = FlameContext::get_context();
	fct->log()->ltrace("layout", "It should be here one time!!!!");
    while (true) {
        if (it_ == csd_info_.end()) {
            if (flag == false){ //初始化过后没有更新layout所以这里有一次填的过程
            	fct->log()->ltrace("layout","flag = %d",flag);
            	fct->log()->ltrace("layout","reach the first csd_info_.end()初始化什么都没有，应该第一次到达此处");
				flag = true;
			}
            else 
                return RC_FAILD;
            
            csdm_->read_lock();
            for (auto it = csdm_->csd_hdl_begin(); it != csdm_->csd_hdl_end(); ++it) {
                CsdObject* obj = it->second->read_and_lock();

                if (obj == nullptr)
                    continue;

                if (csd_info_.find(obj->get_csd_id()) == csd_info_.end()) {// if not found
               		fct->log()->ltrace("layout","insert one map:(csd_id:%llu,csd_unused:%llu-%llu) obj->getname():%s",obj->get_csd_id(),obj->get_size(),obj->get_alloced(),obj->get_name().c_str());
                    csd_info_[obj->get_csd_id()] = obj->get_size() - obj->get_alloced();
                } else if (!it->second->is_active()) {
					fct->log()->ltrace("layout","erase:(csd_id:%llu,csd_unused:%d) because is unactive",obj->get_csd_id(),obj->get_size() - obj->get_alloced());
                    csd_info_.erase(obj->get_csd_id());
                }

                it->second->unlock();
            }
            it_ = csd_info_.begin();
            csdm_->unlock();
        }
        fct->log()->ltrace("layout","it_ != csd_info_.end()");
        fct->log()->ltrace("layout","it_->first=%llu,it_->second=%llu,chk_sz=%llu,csdm_->find(it_->first)->is_active()=%d",it_->first,it_->second,chk_sz,csdm_->find(it_->first)->is_active());
		fct->log()->ltrace("layout","it_->second=%llu",it_->second);
        if (it_->second >= chk_sz && csdm_->find(it_->first)->is_active()) {
            csd_id = it_->first;
            csd_info_[csd_id] -= chk_sz;
            ++it_;
			fct->log()->ltrace("layout","get csd_id : %d",csd_id);
            return RC_SUCCESS;
        } else {
            ++it_;
        }
    }    
}

int PollLayout::select(std::list<uint64_t>& csd_ids, int num, uint64_t chk_sz) {
    WriteLocker locker(select_lock_);
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
    WriteLocker locker(select_lock_);
	FlameContext* fct = FlameContext::get_context();
	fct->log()->ltrace("layout","grp=%d,cgn=%d",grp,cgn);
	int grp_tmp = grp,cgn_tmp = cgn;
	int times = 0;
    while (grp_tmp--) {
        while (cgn_tmp--) {
            uint64_t csd_id;
            if (get_next_csd(csd_id, chk_sz) == RC_SUCCESS) {
                csd_ids.push_back(csd_id);
				times++;
				fct->log()->ltrace("layout","times=%d",times);
            } else {
                return RC_FAILD;
            }
        }
		cgn_tmp = cgn;
    }

    return RC_SUCCESS;
}

} // namespace layout
} // namespace flame
