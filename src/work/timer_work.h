#ifndef FLAME_WORK_WORKER_H
#define FLAME_WORK_WORKER_H

#include "work/work_base.h"
#include "common/thread/rw_lock.h"
#include "util/utime.h"

#include <string>
#include <memory>
#include <map>

namespace flame {

class TimerWorker : public WorkerBase {
public:
    TimerWorker(const utime_t& cycle = utime_t::get_by_msec(500)) 
    : WorkerBase("timer_worker"), check_cycle_(cycle) {}

    ~TimerWorker() {}

    virtual void entry() override;

    void push_cycle(const std::shared_ptr<WorkEntry>& we, const utime_t& interval, bool imm = false);
    void push_timing(const std::shared_ptr<WorkEntry>& we, const utime_t& attack);
    void push_delay(const std::shared_ptr<WorkEntry>& we, const utime_t& delay);

    size_t size() const { return wait_.size(); }

    class TimerEntry {
    public:
        TimerEntry(const std::shared_ptr<WorkEntry>& we, uint64_t interval = 0)
        : we_(we), interval_(interval) {}

        bool is_cycle() const { return interval_ == 0; }

        std::shared_ptr<WorkEntry> we_;

        uint64_t interval_; // (ns)
    }; // class TimerEntry
private:
    void insert__(uint64_t t, const TimerEntry& te);
    std::multimap<uint64_t, TimerEntry>::const_iterator get_top__();
    void erase__(const std::multimap<uint64_t, TimerWorker::TimerEntry>::const_iterator& it);

    std::multimap<uint64_t, TimerEntry> wait_;
    RWLock wait_lock_;
    utime_t check_cycle_;
}; // class TimerWorker

} // namespace flame

#endif // !FLAME_WORK_WORKER_H