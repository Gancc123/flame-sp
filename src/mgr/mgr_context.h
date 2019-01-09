#ifndef FLAME_MGR_CONTEXT_H
#define FLAME_MGR_CONTEXT_H

#include "common/context.h"
#include "metastore/ms.h"
#include "mgr/csdm/csd_mgmt.h"
#include "mgr/chkm/chk_mgmt.h"
#include "mgr/volm/vol_mgmt.h"

#include "cluster/clt_mgmt.h"
#include "work/timer_work.h"

#include <memory>

namespace flame {

class MgrContext {
public:
    MgrContext(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }
    
    std::shared_ptr<MetaStore> ms() const { return ms_; }
    void ms(const std::shared_ptr<MetaStore>& msv) { ms_ = msv; }

    std::shared_ptr<CsdManager> csdm() const { return csdm_; }
    void csdm(const std::shared_ptr<CsdManager>& csdm) { csdm_ = csdm; }

    std::shared_ptr<ClusterMgmt> cltm() const { return cltm_; }
    void cltm(const std::shared_ptr<ClusterMgmt>& cltm) { cltm_ = cltm; }

    std::shared_ptr<ChunkManager> chkm() const { return chkm_; }
    void chkm(std::shared_ptr<ChunkManager>& chkm) { chkm_ = chkm; }

    std::shared_ptr<VolumeManager> volm() const { return volm_; }
    void volm(std::shared_ptr<VolumeManager>& volm) { volm_ = volm; }

    std::shared_ptr<TimerWorker> timer() const { return timer_; }
    void timer(const std::shared_ptr<TimerWorker>& tw) { timer_ = tw; }

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    std::shared_ptr<CsdManager> csdm_;
    std::shared_ptr<ClusterMgmt> cltm_;
    std::shared_ptr<ChunkManager> chkm_;
    std::shared_ptr<VolumeManager> volm_;

    std::shared_ptr<TimerWorker> timer_;
}; // class MgrContext

} // namespace flame

#endif // FLAME_MGR_CONTEXT_H