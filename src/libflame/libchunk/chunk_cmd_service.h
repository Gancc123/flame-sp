/*
 * @Descripttion: 声明并定义四种对chunk的操作函数:read, write, read_zeros, write_zeros
 * @version: 
 * @Author: liweiguang
 * @Date: 2019-05-13 15:07:59
 * @LastEditors: liweiguang
 * @LastEditTime: 2019-05-20 10:56:27
 */
#ifndef FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H
#define FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H
#include "include/cmd.h"

#include <iostream>
#include <cstring>

#include "include/csdc.h"
#include "include/retcode.h"
#include "msg/msg_core.h"
#include "libflame/libchunk/msg_handle.h"

namespace flame {
    
inline int chunk_io_rw_mem(chk_id_t chunk_id, chk_off_t offset, uint32_t len, uint64_t laddr, bool rw){
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
    inline int call(RdmaWorkRequest *req) override{
        msg::Connection* conn = req->conn;
        msg::RdmaConnection* rdma_conn = msg::RdmaStack::rdma_conn_cast(conn);
        ChunkReadCmd* cmd_chunk_read = new ChunkReadCmd((cmd_t *)req->command);
        cmd_ma_t& ma = ((cmd_chk_io_rd_t *)cmd_chunk_read->get_content())->ma;
        FlameContext* flame_context = FlameContext::get_context();
        if(req->stage == 0){                 //**创建rdma内存，并从底层chunkstore异步读取数据到指定的rdma buffer**//          
            auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
            
            flame_context->log()->ltrace("ma_len = %llu", cmd_chunk_read->get_ma_len());
            msg::ib::RdmaBuffer* lbuf = allocator->alloc(cmd_chunk_read->get_ma_len()); //获取一片本地的内存
            lbuf->data_len = cmd_chunk_read->get_ma_len();
            req->buf_ = lbuf;
            
            chunk_io_rw_mem(cmd_chunk_read->get_chk_id(),cmd_chunk_read->get_off(), cmd_chunk_read->get_ma_len(), (uint64_t)lbuf->buffer(), 0); //read，将数据读到lbuf，然后RDMA_write到rbuf
            // flame_context->log()->ltrace("rdma write done. buf: %c...%c %lluB" ,
            //     lbuf->buffer()[0], lbuf->buffer()[1],
            //     lbuf->data_len);
            req->stage++;
        }else if(req->stage == 1){           //** 新建req进行read/write**//
            RdmaWorkRequest* rw_req = req->msger_->get_req_pool().alloc_req();

            flame_context->log()->ltrace("req->buf_->addr() = %llu", req->buf_->addr());
            flame_context->log()->ltrace("req->buf_->size() = %llu", req->buf_->size());
            flame_context->log()->ltrace("req->buf_->lkey() = %llu", req->buf_->lkey());
            flame_context->log()->ltrace("ma.addr = %llu"          , ma.addr);
            flame_context->log()->ltrace("ma.key = %llu"           , ma.key);

            rw_req->buf_ = req->buf_;
            rw_req->sge_.addr = req->buf_->addr();
            rw_req->sge_.length = req->buf_->size();
            rw_req->sge_.lkey = req->buf_->lkey();
            
            rw_req->send_wr_.wr.rdma.remote_addr = ma.addr;
            rw_req->send_wr_.wr.rdma.rkey = ma.key;

            memset(&rw_req->send_wr_, 0, sizeof(rw_req->send_wr_));
            rw_req->send_wr_.wr_id = reinterpret_cast<uint64_t>((msg::RdmaSendWr *)rw_req);
            rw_req->send_wr_.num_sge = 1;
            rw_req->send_wr_.sg_list = &rw_req->sge_;
            rw_req->send_wr_.opcode = IBV_WR_RDMA_WRITE;
            rw_req->send_wr_.next = nullptr;

            rw_req->conn = rdma_conn;
            
            rdma_conn->post_send(rw_req);
            rw_req->stage = 2;
            
        }else if(req->stage == 2){         //**发送response**//
            req->send_wr_.opcode = IBV_WR_SEND;
            req->send_wr_.send_flags |= IBV_SEND_SIGNALED;
            cmd_rc_t rc = 0;
            req->command = new (req->command) ChunkReadRes(*(Command *)req->command, rc); //**这里有问题，第二个read/write的req没有头了
            rdma_conn->post_send(req);
            req->stage++;
        }else{                              //**发送完response之后不需要任何操作**//
            return 0;
        }
        return 0;

    }

    ReadCmdService():CmdService(){}
    
    virtual ~ReadCmdService() {}
}; // class ReadCmdService


// class WriteCmdService final : public CmdService {
// public:
//     inline virtual int call(msg::Connection* connection, const Command& command) override{
//         FlameContext* flame_context = FlameContext::get_context();
//         msg::MsgContext* mct = connection->get_session()->get_msg_context();
//         cmd_t cmd = command.get_cmd();
//         ChunkWriteCmd* cmd_chunk_write = new ChunkWriteCmd(&cmd);
//         auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
//         msg::ib::RdmaBuffer* lbuf = allocator->alloc(cmd_chunk_write->get_ma_len()); //获取一片本地的内存
//         lbuf->data_len = cmd_chunk_write->get_ma_len();
        
//         chunk_io_rw_mem(cmd_chunk_write->get_chk_id(),cmd_chunk_write->get_off(), cmd_chunk_write->get_ma_len(), (uint64_t)lbuf->buffer(), 1);

//         auto rdma_cb = new msg::RdmaRwWork();
//         assert(rdma_cb);
//         rdma_cb->target_func = 
//                             [this, flame_context, mct, cmd, allocator](msg::RdmaRwWork *w, msg::RdmaConnection *conn){   
//             msg::ib::RdmaBuffer * lbuf = w->lbufs[0];
//             msg::ib::RdmaBuffer* rbuf = w->rbufs[0];
//             // ML(mct, info, "rbuf addr {}",(uint64_t)rbuf->buffer());

//             // ML(mct, info, "rdma write done. buf: {}...{} {}B",
//             //     lbuf->buffer()[0], lbuf->buffer()[1],
//             //     lbuf->data_len);
//             allocator->free_buffers(w->lbufs);
//             // ML(mct, info, "rdma write done. bufB");
//             allocator->free_buffers(w->rbufs);   
//             // ML(mct, info, "rdma write done. bufB");
//             delete w;
//             /**发送read response消息*/
//             CommonRes* common_res = new CommonRes(cmd, 0);
//             MsgCmd msg_request_cmd;
//             common_res->copy(&msg_request_cmd.cmd);
//             msg::Msg* req_msg = msg::Msg::alloc_msg(mct);
//             req_msg->append_data(msg_request_cmd);
//             conn->send_msg(req_msg);
//             req_msg->put();
//         };
//         msg::ib::RdmaBuffer *rbuf = new msg::ib::RdmaBuffer(cmd_chunk_write->get_ma_key(), cmd_chunk_write->get_ma_addr(), cmd_chunk_write->get_ma_len());
//         rbuf->data_len = cmd_chunk_write->get_ma_len();
//         rdma_cb->is_write = false;
//         rdma_cb->rbufs.push_back(rbuf);
//         rdma_cb->lbufs.push_back(lbuf);
//         rdma_cb->cnt = 1;

//         auto rdma_conn = msg::RdmaStack::rdma_conn_cast(connection);
//         assert(rdma_conn);
//         rdma_conn->post_rdma_rw(rdma_cb);

//         return 0;
//     }

//     WriteCmdService() : CmdService(){}

//     virtual ~WriteCmdService() {}
// }; // class WriteCmdService


class ReadZerosCmdService final : public CmdService {
public:
    inline virtual int call(RdmaWorkRequest *req) override{
        return 0;
    }

    ReadZerosCmdService() : CmdService(){}

    virtual ~ReadZerosCmdService() {}
}; // class ReadZerosCmdService


class WriteZerosCmdService final : public CmdService {
public:
    inline virtual int call(RdmaWorkRequest *req) override{
        return 0;
    }

    WriteZerosCmdService() : CmdService(){}

    virtual ~WriteZerosCmdService() {}
}; // class WriteZerosCmdService

} // namespace flame

#endif //FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H