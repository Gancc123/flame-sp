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

    std::shared_ptr<ChunkStore> cs() const { return cs_; }
    void cs(std::shared_ptr<ChunkStore>& csv) { cs_ = csv; }

private:
    FlameContext* fct_;
    std::shared_ptr<ChunkStore> cs_;
}; // class CsdContext

} // namespace flame

#endif // FLAME_CSD_CONTEXT_H