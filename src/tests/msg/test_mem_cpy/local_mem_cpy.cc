#include "msg/msg_core.h"
#include "util/clog.h"

#include "get_clock.h"

#include <unistd.h>
#include <cstring>
#include <vector>
#include <fstream>

using namespace flame;

#define LEVEL_MASK(l) ((-1) >> (l) << (l))

void measure_mem_cpy(std::vector<msg::ib::RdmaBuffer *> &bufs, size_t ms, 
                     int level_begin, int level_end, int cnt_per_round, 
                     const std::string &out_f){
    double cycle_to_unit = get_cpu_mhz(0); // no warn
    cycles_t start, end;
    double avg_sum = 0;
    std::ofstream f;
    f.open(out_f);
    for(int l = level_begin;l <= level_end;++l){
        f << "LEVEL: " << l << '\n';
        avg_sum = 0;
        for(int cnt = 0; cnt < cnt_per_round;++cnt){
            size_t sel = msg::gen_rand_seq() % ms;
            sel = sel & LEVEL_MASK(l);
            int buf_index = cnt % (bufs.size() / 2);
            char *src = bufs[buf_index]->buffer() + sel;
            char *dst = bufs[buf_index + 1]->buffer() + sel;
            start = get_cycles();
            std::memcpy(dst, src, 1 << l);
            end = get_cycles();
            double t = (end - start) / cycle_to_unit;
            f << sel << ':' << t << '\n';
            avg_sum += t;
        }
        f << "\tAVG: " << (avg_sum / cnt_per_round) << "us\n";
    }

    f.close();
}



int main(){

    auto fct = FlameContext::get_context();
    fct->init_config("flame_mgr.cfg");
    fct->init_log("", "TRACE", "local_mem_cpy");

    auto mct = new msg::MsgContext(fct);

    if(mct->load_config()){
        return 0;
    }

    //config BuddyAllocator
    mct->config->rdma_mem_min_level = 14; // 16KB
    mct->config->rdma_mem_max_level = 30; // 1GB

    if(mct->init(nullptr, nullptr)){
        return 0;
    }

    auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();

    std::vector<msg::ib::RdmaBuffer *> bufs;

    allocator->alloc_buffers(1 << 29, 4, bufs); // 512MB

    measure_mem_cpy(bufs, 1 << 29, 6, 28, 10, "local_mem_cpy.txt");

    allocator->free_buffers(bufs);

    mct->fin();
    delete mct;

    return 0;
}