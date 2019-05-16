/*
 * @Descripttion: 声明并定义四种对chunk的操作函数:read, write, read_zeros, write_zeros
 * @version: 
 * @Author: liweiguang
 * @Date: 2019-05-13 15:07:59
 * @LastEditors: liweiguang
 * @LastEditTime: 2019-05-16 19:06:14
 */
#ifndef FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H
#define FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H
#include "include/cmd.h"

#include <iostream>
#include <cstring>

#include "include/csdc.h"
#include "include/retcode.h"
#include "msg/msg_core.h"

namespace flame {
    
int chunk_io_rw_mem(chk_id_t chunk_id, chk_off_t offset, uint32_t len, uint64_t laddr, bool rw){
    if(rw){ //**?
        ;
    }
    else{   //**?
        char *disk = new char[len];
        strcpy(disk,"aaaaaaa");
        memcpy((void *)laddr, disk, len);
    }
    return RC_SUCCESS;
}


class ReadCmdService final : public CmdService {
public:
    inline int call(msg::Connection* connection, cmd_t& cmd) override{
        ChunkReadCmd* cmd_chunk_read = new ChunkReadCmd(&cmd);
        FlameContext* flame_context = FlameContext::get_context();
        msg::MsgContext* mct = connection->get_session()->get_msg_context();
        auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
        msg::ib::RdmaBuffer* lbuf = allocator->alloc(cmd_chunk_read->get_ma_len()); //获取一片本地的内存
        lbuf->data_len = cmd_chunk_read->get_ma_len();
        
        chunk_io_rw_mem(cmd_chunk_read->get_chk_id(),cmd_chunk_read->get_off(), cmd_chunk_read->get_ma_len(), (uint64_t)lbuf->buffer(), 0); //read，将数据读到lbuf，然后RDMA_write到rbuf

        flame_context->log()->ltrace("rdma write done. buf: %c...%c %lluB",lbuf->buffer()[0], 
                lbuf->buffer()[1], lbuf->data_len);

        auto rdma_cb = new msg::RdmaRwWork();
        assert(rdma_cb);
        rdma_cb->target_func = 
                            [this, flame_context, mct, cmd, allocator](msg::RdmaRwWork *w, msg::RdmaConnection *conn){   
            msg::ib::RdmaBuffer * lbuf = w->lbufs[0];
            msg::ib::RdmaBuffer* rbuf = w->rbufs[0];
            flame_context->log()->ltrace("rbuf addr %llu",(uint64_t)rbuf->buffer()); 

            flame_context->log()->ltrace("rdma write done. buf: %c...%c %lluB" ,
                lbuf->buffer()[0], lbuf->buffer()[1],
                lbuf->data_len);
            
            allocator->free_buffers(w->lbufs);
            flame_context->log()->ltrace("rdma write done. bufB"); 
            allocator->free_buffers(w->rbufs);   
            delete w;
            /**发送read response消息*/
            ChunkReadRes* chunk_read_res = new ChunkReadRes(cmd, 0);
            MsgCmd msg_request_cmd;
            chunk_read_res->copy(&msg_request_cmd.cmd);
            msg::Msg* req_msg = msg::Msg::alloc_msg(mct);
            req_msg->append_data(msg_request_cmd);
            conn->send_msg(req_msg);
            req_msg->put();
        };
        msg::ib::RdmaBuffer *rbuf = new msg::ib::RdmaBuffer(cmd_chunk_read->get_ma_key(), cmd_chunk_read->get_ma_addr(), cmd_chunk_read->get_ma_len());
        rbuf->data_len = cmd_chunk_read->get_ma_len();
        rdma_cb->is_write = true;
        rdma_cb->rbufs.push_back(rbuf);
        rdma_cb->lbufs.push_back(lbuf);
        rdma_cb->cnt = 1;

        auto rdma_conn = msg::RdmaStack::rdma_conn_cast(connection);
        assert(rdma_conn);
        rdma_conn->post_rdma_rw(rdma_cb);

        return 0;
    }

    ReadCmdService():CmdService(){}
    
    virtual ~ReadCmdService() {}
}; // class ReadCmdService


class WriteCmdService final : public CmdService {
public:
    inline virtual int call(msg::Connection* connection, const Command& command) override{
        FlameContext* flame_context = FlameContext::get_context();
        msg::MsgContext* mct = connection->get_session()->get_msg_context();
        cmd_t cmd = command.get_cmd();
        ChunkWriteCmd* cmd_chunk_write = new ChunkWriteCmd(&cmd);
        auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
        msg::ib::RdmaBuffer* lbuf = allocator->alloc(cmd_chunk_write->get_ma_len()); //获取一片本地的内存
        lbuf->data_len = cmd_chunk_write->get_ma_len();
        
        chunk_io_rw_mem(cmd_chunk_write->get_chk_id(),cmd_chunk_write->get_off(), cmd_chunk_write->get_ma_len(), (uint64_t)lbuf->buffer(), 1);

        auto rdma_cb = new msg::RdmaRwWork();
        assert(rdma_cb);
        rdma_cb->target_func = 
                            [this, flame_context, mct, cmd, allocator](msg::RdmaRwWork *w, msg::RdmaConnection *conn){   
            msg::ib::RdmaBuffer * lbuf = w->lbufs[0];
            msg::ib::RdmaBuffer* rbuf = w->rbufs[0];
            // ML(mct, info, "rbuf addr {}",(uint64_t)rbuf->buffer());

            // ML(mct, info, "rdma write done. buf: {}...{} {}B",
            //     lbuf->buffer()[0], lbuf->buffer()[1],
            //     lbuf->data_len);
            allocator->free_buffers(w->lbufs);
            // ML(mct, info, "rdma write done. bufB");
            allocator->free_buffers(w->rbufs);   
            // ML(mct, info, "rdma write done. bufB");
            delete w;
            /**发送read response消息*/
            CommonRes* common_res = new CommonRes(cmd, 0);
            MsgCmd msg_request_cmd;
            common_res->copy(&msg_request_cmd.cmd);
            msg::Msg* req_msg = msg::Msg::alloc_msg(mct);
            req_msg->append_data(msg_request_cmd);
            conn->send_msg(req_msg);
            req_msg->put();
        };
        msg::ib::RdmaBuffer *rbuf = new msg::ib::RdmaBuffer(cmd_chunk_write->get_ma_key(), cmd_chunk_write->get_ma_addr(), cmd_chunk_write->get_ma_len());
        rbuf->data_len = cmd_chunk_write->get_ma_len();
        rdma_cb->is_write = false;
        rdma_cb->rbufs.push_back(rbuf);
        rdma_cb->lbufs.push_back(lbuf);
        rdma_cb->cnt = 1;

        auto rdma_conn = msg::RdmaStack::rdma_conn_cast(connection);
        assert(rdma_conn);
        rdma_conn->post_rdma_rw(rdma_cb);

        return 0;
    }

    WriteCmdService() : CmdService(){}

    virtual ~WriteCmdService() {}
}; // class WriteCmdService


class ReadZerosCmdService final : public CmdService {
public:
    inline virtual int call(msg::Connection* connection, const Command& cmd) override{
        return 0;
    }

    ReadZerosCmdService() : CmdService(){}

    virtual ~ReadZerosCmdService() {}
}; // class ReadZerosCmdService


class WriteZerosCmdService final : public CmdService {
public:
    inline virtual int call(msg::Connection* connection, const Command& cmd) override{
        return 0;
    }

    WriteZerosCmdService() : CmdService(){}

    virtual ~WriteZerosCmdService() {}
}; // class WriteZerosCmdService

} // namespace flame

#endif //FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H