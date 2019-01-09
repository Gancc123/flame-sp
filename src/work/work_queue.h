#ifndef FLAME_WORK_QUEUE_H
#define FLAME_WORK_QUEUE_H

#include "work/work_base.h"
#include "common/atom_queue.h"
#include "common/thread/mutex.h"
#include "common/thread/cond.h"

#include <cstdint>
#include <memory>

namespace flame {

class WorkQueue : public WorkerBase {
public:
    static const int max_try_times = 16;

    WorkQueue(size_t cap) : WorkerBase("work_queue") 
    : ring_(cap, max_try_times) {

    }

    ~WorkQueue() {}

    virtual void entry() override;

    void push(const std::shared_ptr<WorkEntry>& we);

    size_t size() const { return ring_.size(); }

private:
    RingQueue<std::shared_ptr<WorkEntry>> ring_;
    Mutex wq_mutex_;
    Cond wq_cond_ {wq_mutex_};
}; // class WorkQueue

} // namespace flame

#endif // FLAME_WORK_QUEUE_H