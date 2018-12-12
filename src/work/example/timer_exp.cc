#include "work/timer_work.h"
#include "common/atom_queue.h"
#include "util/utime.h"

#include <cstdio>
#include <memory>
#include <list>
#include <map>

using namespace std;
using namespace flame;

class PrinterWork : public WorkEntry {
public:
    virtual void entry() override {
        static int idx = 0;
        idx++;
        printf("Printer: %s  %d\n", utime_t::now().to_str().c_str(), idx);
    }
}; // class PrinterWork

class TriggerWork : public WorkEntry {
public:
    virtual void entry() override {
        static int idx = 0;
        idx++;
        printf("Trigger: %s  %d\n", utime_t::now().to_str().c_str(), idx);
    }
}; // class TriggerWork

void timer_exp() {
    utime_t cycle = utime_t::get_by_msec(1000);
    TimerWorker tw(cycle);
    tw.run();

    tw.push_cycle(std::shared_ptr<WorkEntry>(new PrinterWork()), utime_t::get_by_msec(3000));
    tw.push_delay(std::shared_ptr<WorkEntry>(new TriggerWork()), utime_t::get_by_msec(10000));

    tw.join();
}

int main() {
    timer_exp();
}