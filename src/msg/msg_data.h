#ifndef FLAME_MSG_SIMPLE_ADDR_MANAGER_ADDR_MSG_H
#define FLAME_MSG_SIMPLE_ADDR_MANAGER_ADDR_MSG_H

#include "common/context.h"
#include "internal/int_types.h"
#include "internal/types.h"
#include "internal/byteorder.h"
#include "internal/msg_buffer_list.h"
#include "msg_types.h"

#include <vector>

#ifdef HAVE_RDMA
    #include "msg/rdma/RdmaMem.h"
#endif

#define M_ENCODE(bl, data) (bl).append(&(data), sizeof(data))
#define M_DECODE(it, data) (it).copy(&(data), sizeof(data))

namespace flame{

#ifdef HAVE_RDMA
struct msg_rdma_header_d : public MsgData{
    using RdmaBuffer = ib::RdmaBuffer;
    uint8_t cnt;
    std::vector<RdmaBuffer *> rdma_bufs;
    explicit msg_rdma_header_d(uint8_t c=0) : cnt(c) {};
    
    void clear(){
        cnt = 0;
        for(auto buffer : rdma_bufs){
            delete buffer;
        }
        rdma_bufs.clear();
    }

    virtual size_t size() override {
        return sizeof(flame_msg_rdma_mr_t) * cnt;
    }

    virtual int encode(MsgBufferList& bl) override{
        int total = 0;
        flame_msg_rdma_mr_t mr;
        for(int i = 0;i < rdma_bufs.size();++i){
            mr.addr = rdma_bufs[i]->addr();
            mr.rkey = rdma_bufs[i]->rkey();
            mr.len = rdma_bufs[i]->data_len;
            total += M_ENCODE(bl, mr);
        }
        return total;
    }

    virtual int decode(MsgBufferList::iterator& it) override{
        flame_msg_rdma_mr_t mr;
        int total = 0;
        for(int i = 0;i < cnt;++i){
            int len = M_DECODE(it, mr);
            assert(len == sizeof(mr));
            total += len;
            auto buf = new RdmaBuffer(mr.rkey, mr.addr, mr.len);
            assert(buf);
            rdma_bufs.push_back(buf);
        }
        return total;
    }
};
#endif //HAVE_RDMA

//* 这里没有考虑大小端问题，如果要移植到大端机，此处需修改。
struct msg_declare_id_d : public MsgData{
    msger_id_t msger_id;
    bool has_tcp_lp = false;
    bool has_rdma_lp = false;
    node_addr_t tcp_listen_addr;
    node_addr_t rdma_listen_addr;

    virtual size_t size() override {
        return sizeof(msger_id_t) + sizeof(bool) * 2 + sizeof(node_addr_t) * 2;
    }

    virtual int encode(MsgBufferList& bl) override{
        int write_len = 0;
        write_len += M_ENCODE(bl, msger_id);
        write_len += M_ENCODE(bl, has_tcp_lp);
        write_len += M_ENCODE(bl, has_rdma_lp);
        if(has_tcp_lp){
            write_len += M_ENCODE(bl, tcp_listen_addr);
        }
        if(has_rdma_lp){
            write_len += M_ENCODE(bl, rdma_listen_addr);
        }
        return write_len;
    }

    virtual int decode(MsgBufferList::iterator& it) override{
        int read_len = 0;
        
        read_len += M_DECODE(it, msger_id);
        read_len += M_DECODE(it, has_tcp_lp);
        read_len += M_DECODE(it, has_rdma_lp);
        if(has_tcp_lp){
            read_len += M_DECODE(it, tcp_listen_addr);
        }
        if(has_rdma_lp){
            read_len += M_DECODE(it, rdma_listen_addr);
        }
        return read_len;
    }
};



}

#undef M_ENCODE
#undef M_DECODE

#endif 