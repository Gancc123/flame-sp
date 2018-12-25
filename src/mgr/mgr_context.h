#ifndef FLAME_MGR_CONTEXT_H
#define FLAME_MGR_CONTEXT_H

#include "common/context.h"
#include "metastore/ms.h"
#include "mgr/csdm/csd_mgmt.h"
#include "mgr/chunkm/chunk_mgmt.h"
#include "mgr/volm/vol_mgmt.h"

#include <memory>

namespace flame {

class MgrContext {
public:
    MgrContext(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }
    
    std::shared_ptr<MetaStore> ms() const { return ms_; }
    void ms(std::shared_ptr<MetaStore>& msv) { ms_ = msv; }

    std::shared_ptr<CsdManager> csdm() const { return csdm_; }
    void csdm(std::shared_ptr<CsdManager>& csdm) { csdm_ = csdm; }

    std::shared_ptr<ChkManager> chkm() const { return chkm_; }
    void chkm(std::shared_ptr<ChkManager>& chkm) { chkm_ = chkm; }

    std::shared_ptr<VolManager> volm() const { return volm_; }
    void volm(std::shared_ptr<VolManager>& volm) { volm_ = volm; }

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    std::shared_ptr<CsdManager> csdm_;
    std::shared_ptr<ChkManager> chkm_;
    std::shared_ptr<VolManager> volm_;
}; // class MgrContext

} // namespace flame

#endif // FLAME_MGR_CONTEXT_H