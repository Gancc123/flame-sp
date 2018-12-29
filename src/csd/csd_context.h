#ifndef FLAME_CSD_CONTEXT_H
#define FLAME_CSD_CONTEXT_H

#include "common/context.h"
#include "chunkstore/cs.h"
#include "work/timer_work.h"
#include "include/meta.h"
#include "include/internal.h"

#include <atomic>
#include <cstdint>

namespace flame {

class CsdContext {
public:
    CsdContext(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }

    uint64_t csd_id() const { return csd_id_; }
    void csd_id(uint64_t id) { csd_id_ = id; }

    std::string clt_name() const { return clt_name_; }
    void clt_name(const std::string& name) { clt_name_ = name; }

    std::string csd_name() const { return csd_name_; }
    void csd_name(const std::string& name) { csd_name_ = name; }

    node_addr_t admin_addr() const { return admin_addr_; }
    void admin_addr(node_addr_t addr) { admin_addr_ = addr; }

    node_addr_t io_addr() const { return io_addr_; }
    void io_addr(node_addr_t addr) { io_addr_ = addr; }

    uint32_t csd_stat() const { return csd_stat_.load(); }
    bool csd_stat(uint32_t expected, uint32_t val) { 
        return csd_stat_.compare_exchange_strong(expected, val) ? val : expected;
    }

    std::shared_ptr<ChunkStore> cs() const { return cs_; }
    void cs(const std::shared_ptr<ChunkStore>& csv) { cs_ = csv; }

    std::shared_ptr<InternalClient> mgr_stub() const { return mgr_stub_; }
    void mgr_stub(const std::shared_ptr<InternalClient>& stub) { mgr_stub_ = stub; }

    std::shared_ptr<TimerWorker> timer() const { return timer_; }
    void timer(const std::shared_ptr<TimerWorker>& tw) { timer_ = tw; } 

private:
    FlameContext* fct_;

    uint64_t csd_id_;
    std::string clt_name_;
    std::string csd_name_;
    node_addr_t admin_addr_;
    node_addr_t io_addr_;
    std::atomic<uint32_t> csd_stat_ {CSD_STAT_NONE};

    std::shared_ptr<ChunkStore> cs_;
    std::shared_ptr<InternalClient> mgr_stub_;
    std::shared_ptr<TimerWorker> timer_;
}; // class CsdContext

} // namespace flame

#endif // FLAME_CSD_CONTEXT_H