#ifndef FLAME_LAYOUT_POLL_H
#define FLAME_LAYOUT_POLL_H

#include "layout.h"
#include <map>
#include <set>

namespace flame  {
namespace layout {

class PollLayout;

class PollWork : public WorkEntry {
public:
    PollWork(PollLayout* playout)
    : playout_(playout) {}

    virtual void entry() override;

private:
    PollLayout* playout_;
}; // class PollWork

class PollLayout : public ChunkLayout {
public:
    PollLayout(const std::shared_ptr<CsdManager>& csdm, const std::shared_ptr<TimerWorker>& tw, const utime_t& cycle)
    : ChunkLayout(csdm), tw_(tw), cycle_(cycle) {}

    void init(); // 设置定时更新csd信息

    void set_csd_info();

    int get_next_csd(uint64_t& csd_id, const uint64_t& chk_sz);

    virtual int select(std::list<uint64_t>& csd_ids, int num, uint64_t chk_sz) override;

    virtual int select_bulk(std::list<uint64_t>& csd_ids, int grp, int cgn, uint64_t chk_sz) override;

private:
    std::map<uint64_t, uint64_t> csd_info_;
    std::map<uint64_t, uint64_t>::iterator it_;
    RWLock csd_info_lock_;
    std::shared_ptr<TimerWorker> tw_;
    utime_t cycle_;

};

}
}


#endif