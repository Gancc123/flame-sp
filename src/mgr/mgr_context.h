#ifndef FLAME_MGR_CONTEXT_H
#define FLAME_MGR_CONTEXT_H

#include "common/context.h"
#include "metastore/ms.h"

#include <memory>

namespace flame {

class MgrContext {
public:
    MgrContext(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->log(); }
    
    std::shared_ptr<MetaStore> ms() const { return ms_; }
    void ms(std::shared_ptr<MetaStore>& msv) { ms_ = msv; }

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
}; // class MgrContext

} // namespace flame

#endif // FLAME_MGR_CONTEXT_H