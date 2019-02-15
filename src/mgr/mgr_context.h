#ifndef FLAME_MGR_CONTEXT_H
#define FLAME_MGR_CONTEXT_H

#include "common/context.h"
#include "metastore/ms.h"
#include "spolicy/spolicy.h"
#include "work/timer_work.h"

#include <memory>
#include <vector>

namespace flame {

class CsdManager;
class ChunkManager;
class VolumeManager;
class ClusterMgmt;

class MgrBaseContext {
public:
    MgrBaseContext(FlameContext* fct) : fct_(fct) {
        create_spolicy_bulk(sp_tbl_);
    }

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }

    /**
     * MetaStore
     */
    std::shared_ptr<MetaStore> ms() const { return ms_; }
    void ms(const std::shared_ptr<MetaStore>& msv) { ms_ = msv; }

    /**
     * StorePolicy
     */
    spolicy::StorePolicy* get_spolicy(uint32_t sp_type) const { 
        return sp_type < sp_tbl_.size() ? sp_tbl_[sp_type] : nullptr;
    }
    const std::vector<spolicy::StorePolicy*>& sp_table() const { return sp_tbl_; }

    /**
     * TimerWorker
     */
    std::shared_ptr<TimerWorker> timer() const { return timer_; }
    void timer(const std::shared_ptr<TimerWorker>& tw) { timer_ = tw; }

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    std::vector<spolicy::StorePolicy*> sp_tbl_;
    std::shared_ptr<TimerWorker> timer_;
}; // MgrBaseContext

class MgrContext {
public:
    MgrContext(MgrBaseContext* bct) : bct_(bct) {}

    MgrBaseContext* bct() const { return bct_; }
    FlameContext* fct() const { return bct_->fct(); }
    Logger* log() const { return bct_->fct()->log(); }
    FlameConfig* config() const { return bct_->fct()->config(); }
    
    std::shared_ptr<MetaStore> ms() const { return bct_->ms(); }
    void ms(const std::shared_ptr<MetaStore>& msv) { bct_->ms(msv); }

    std::shared_ptr<TimerWorker> timer() const { return bct_->timer(); }
    void timer(const std::shared_ptr<TimerWorker>& tw) { bct_->timer(tw); }

    std::shared_ptr<CsdManager> csdm() const { return csdm_; }
    void csdm(const std::shared_ptr<CsdManager>& csdm) { csdm_ = csdm; }

    std::shared_ptr<ClusterMgmt> cltm() const { return cltm_; }
    void cltm(const std::shared_ptr<ClusterMgmt>& cltm) { cltm_ = cltm; }

    std::shared_ptr<ChunkManager> chkm() const { return chkm_; }
    void chkm(std::shared_ptr<ChunkManager>& chkm) { chkm_ = chkm; }

    std::shared_ptr<VolumeManager> volm() const { return volm_; }
    void volm(std::shared_ptr<VolumeManager>& volm) { volm_ = volm; }

private:
    MgrBaseContext* bct_;
    std::shared_ptr<CsdManager> csdm_;
    std::shared_ptr<ClusterMgmt> cltm_;
    std::shared_ptr<ChunkManager> chkm_;
    std::shared_ptr<VolumeManager> volm_;
}; // class MgrContext

} // namespace flame

#endif // FLAME_MGR_CONTEXT_H