#include "chunkstore/nvmestore/nvmestore.h"
#include "util/utime.h"
#include "chunkstore/log_cs.h"

using namespace flame;

/*
 * ********************************************************
 * nvmestore_io_channel的成员函数
*/
void nvmestore_io_channel::curr_io_ops_add(uint32_t val) {
    curr_io_ops.fetch_add(val, std::memory_order_relaxed);
    group->curr_io_ops_add(val);
}

void nvmestore_io_channel::curr_io_ops_sub(uint32_t val) {
    curr_io_ops.fetch_sub(val, std::memory_order_relaxed);
    group->curr_io_ops_sub(val);
}

uint64_t nvmestore_io_channel::get_curr_io_ops() const {
    return curr_io_ops.load(std::memory_order_relaxed);
}

void nvmestore_io_channel::alloc(NvmeStore *_store) {
    struct alloc_ch_arg *ch_arg = new alloc_ch_arg(_store, this, nullptr);
    struct spdk_event *event = spdk_event_allocate(lcore, alloc_io_channel, ch_arg, nullptr);
    spdk_event_call(event);

    return ;
}

void nvmestore_io_channel::free() {
    if(io_channel)
        spdk_bs_free_io_channel(io_channel);
    
    lcore = UINT32_MAX;
    curr_io_ops.store(0, std::memory_order_relaxed);

    group = nullptr;
}

void nvmestore_io_channel::alloc_io_channel(void *arg1, void *arg2) {
    struct alloc_ch_arg *ch_arg = (struct alloc_ch_arg *)arg1;

    NvmeStore *nvmestore = ch_arg->nvmestore;
    struct nvmestore_io_channel *channel = ch_arg->io_channel;

    channel->io_channel = spdk_bs_alloc_io_channel(nvmestore->get_blobstore());
    if(channel->io_channel == nullptr) {
        nvmestore->get_fct()->log()->lerror("alloc channel failed on core %ld", channel->lcore);
    } else {
        nvmestore->get_fct()->log()->linfo("curr core = %ld", spdk_env_get_current_core());
        nvmestore->get_fct()->log()->linfo("alloc channel success on core %ld", channel->lcore);
    }

    if(ch_arg)
        delete ch_arg;

    channel->group->signal_alloc_channel_completed();

    nvmestore->get_fct()->log()->linfo("curr core = %ld", spdk_env_get_current_core());
}

/*
 * ****************************************************************************
 * 后面是IOChannels成员函数
*/

void IOChannels::alloc_channels(NvmeStore *_store) {
    for(size_t i = 0; i < channel_nums; i++) {
        uint32_t target_core = _store->get_io_core(type, i);
            
        struct nvmestore_io_channel *channel = new nvmestore_io_channel(this, target_core);
        channels.push_back(channel);

        channel->alloc(_store);
    }

    wait_alloc_channel_completed();
}

void IOChannels::free_channels() {
    for(size_t i = 0; i < channel_nums; i++) {
        struct nvmestore_io_channel *channel = channels[i];
        if(channel)
            channel->free();
    }
}

void IOChannels::add_channel(struct nvmestore_io_channel *channel) {
    std::vector<struct nvmestore_io_channel *>::iterator iter;
    for(iter = channels.begin(); iter != channels.end(); iter++) {
        if(*iter == channel)
            return ;
    }

    channels.push_back(channel);
    channel_nums++;
}

void IOChannels::remove_channel(uint32_t lcore) {
    std::vector<struct nvmestore_io_channel *>::iterator iter;
    for(iter = channels.begin(); iter != channels.end(); iter++) {
        if((*iter)->get_core() == lcore) {
            channels.erase(iter);
            channel_nums--;
            break;
        }
    }
}

void IOChannels::move_channel_to_other_group(struct nvmestore_io_channel *channel, IOChannels *group) {
    std::vector<struct nvmestore_io_channel *>::iterator iter;
    for(iter = channels.begin(); iter != channels.end(); iter++) {
        if((*iter) == channel) {
            group->add_channel(channel);
            channels.erase(iter);
            channel_nums--;
            break;
        }
    }
}

struct spdk_io_channel *IOChannels::get_io_channel_by_index(size_t index) {
    //最好是能够自己算出一个index来
    if(index >= channel_nums) {
        return channels[channel_nums - 1]->get_channel();
    }

    return channels[index]->get_channel();
}

struct spdk_io_channel *IOChannels::get_io_channel_by_core(uint32_t core) {
    std::vector<struct nvmestore_io_channel *>::iterator iter;
    for(iter = channels.begin(); iter != channels.end(); iter++) {
        if((*iter)->get_core() == core) {
            return (*iter)->get_channel();
        }
    }

    return nullptr;
}

struct nvmestore_io_channel *IOChannels::get_nv_channel(uint32_t core) {
    std::vector<struct nvmestore_io_channel *>::iterator iter;
    for(iter = channels.begin(); iter != channels.end(); iter++) {
        if((*iter)->get_core() == core) 
            return *iter;
    }

    return nullptr;
}

struct spdk_io_channel *IOChannels::get_channel() {
        //根据某个特定的策略来确定channel_index, 并获取相应的io_channel
}