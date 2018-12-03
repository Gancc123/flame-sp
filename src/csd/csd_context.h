#ifndef FLAME_CSD_CONTEXT_H
#define FLAME_CSD_CONTEXT_H

#include "common/context.h"
#include "chunkstore/cs.h"

namespace flame {

class CsdContext {
public:
    CsdContext(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }

private:
    FlameContext* fct_;
}; // class CsdContext

} // namespace flame

#endif // FLAME_CSD_CONTEXT_H