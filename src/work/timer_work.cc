#include "work/timer_work.h"

#include <cassert>
#include <iostream>

using namespace std;

namespace flame {

void TimerWorker::entry() {
    while (can_run_.load()) {
        auto it = get_top__();
        if (it != wait_.cend()) {
            utime_t now = utime_t::now();
            if (it->first <= now.to_nsec()) {
                // run it
                it->second.we_->entry();

                if (it->second.interval_ == 0) {
                    // trigger
                    erase__(it);
                    continue;
                } else {
                    // cycle
                    uint64_t next = it->first + it->second.interval_;
                    auto te = it->second;
                    erase__(it);
                    insert__(next, te);
                    continue;
                }
            }
        }
        
        check_cycle_.sleep();
    }
}

void TimerWorker::push_cycle(const shared_ptr<WorkEntry>& we, const utime_t& interval, bool imm) {
    assert(we.get());
    TimerEntry te(we, interval.to_nsec());
    utime_t attack;
    if (imm)
        attack = utime_t::now();
    else
        attack = utime_t::now() + interval;
    
    insert__(attack.to_nsec(), te);
}

void TimerWorker::push_timing(const shared_ptr<WorkEntry>& we, const utime_t& attack) {
    assert(we.get());
    TimerEntry te(we);

    insert__(attack.to_nsec(), te);
}

void TimerWorker::push_delay(const shared_ptr<WorkEntry>& we, const utime_t& delay) {
    assert(we.get());
    TimerEntry te(we);

    utime_t attack = utime_t::now() + delay;

    insert__(attack.to_nsec(), te);
}

void TimerWorker::stop() {
    can_run_.store(false);
    join();
}

void TimerWorker::insert__(uint64_t t, const TimerEntry& te) {
    WriteLocker locker(wait_lock_);
    wait_.insert(pair<uint64_t, TimerEntry>(t, te));
}

multimap<uint64_t, TimerWorker::TimerEntry>::const_iterator TimerWorker::get_top__() {
    ReadLocker locker(wait_lock_);
    return wait_.cbegin();
}

void TimerWorker::erase__(const multimap<uint64_t, TimerEntry>::const_iterator& it) {
    WriteLocker locker(wait_lock_);
    wait_.erase(it);
}
    

} // namespace flame