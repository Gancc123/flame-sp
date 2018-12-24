#ifndef FLAME_CSD_CONTEXT_H
#define FLAME_CSD_CONTEXT_H

#include "common/context.h"
#include "chunkstore/cs.h"
#include "work/timer_work.h"
#include "include/meta.h"
#include "include/internal.h"

namespace flame {

class CsdContext {
public:
    CsdContext(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }

    std::shared_ptr<ChunkStore> cs() const { return cs_; }
    void cs(const std::shared_ptr<ChunkStore>& csv) { cs_ = csv; }

    std::shared_ptr<InternalClient> mgr_stub() const { return mgr_stub_; }
    void mgr_stub(const std::shared_ptr<InternalClient>& stub) { mgr_stub_ = stub; }

    std::shared_ptr<TimerWorker> timer() const { return timer_; }
    void timer(const std::shared_ptr<TimerWorker>& tw) { timer_ = tw; } 

    node_addr_t admin_addr() const { return admin_addr_; }
    void admin_addr(node_addr_t addr) { admin_addr_ = addr; }

    node_addr_t io_addr() const { return io_addr_; }
    void io_addr(node_addr_t addr) { io_addr_ = addr; }

private:
    FlameContext* fct_;
    std::shared_ptr<ChunkStore> cs_;
    std::shared_ptr<InternalClient> mgr_stub_;
    std::shared_ptr<TimerWorker> timer_;

    node_addr_t admin_addr_;
    node_addr_t io_addr_;
}; // class CsdContext

} // namespace flame

#endif // FLAME_CSD_CONTEXT_H