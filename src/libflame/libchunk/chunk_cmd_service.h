/*
 * @Descripttion: 声明并定义四种对chunk的操作函数:read, write, read_zeros, write_zeros
 * @version: 
 * @Author: liweiguang
 * @Date: 2019-05-13 15:07:59
 * @LastEditors: liweiguang
 * @LastEditTime: 2019-05-21 16:52:26
 */
#ifndef FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H
#define FLAME_LIBFLAME_LIBCHUNK_CHUNK_CMD_SERVICE_H
#include "libflame/libchunk/libchunk.h"

#include <iostream>
#include <cstring>

#include "include/csdc.h"
#include "include/retcode.h"
#include "msg/msg_core.h"
#include "libflame/libchunk/msg_handle.h"

namespace flame {
    
/**
 * @name: chunk_io_rw_mem
 * @describtions:  模拟io从硬盘到RDMA内存的过程
 * @param   chk_id_t    chunk_id            chunk的id
 *          chk_off_t   offset              访问chunk的偏移
 *          uint32_t    len                 读/写长度
 *          uint64_t    laddr               本地的RDMA内存
 *          bool        rw                  判断 读/写 标志
 * @return: 
 */
inline int chunk_io_rw_mem(chk_id_t chunk_id, chk_off_t offset, uint32_t len, uint64_t laddr, bool rw){
    if(rw){ //**?
        ;
    }
    else{   //**?
        char *disk = new char[len];
        strcpy(disk,"abcdefghijklmnopq");
        memcpy((void *)laddr, disk, len);
    }
    return RC_SUCCESS;
}


class ReadCmdService final : public CmdService {
public:
    inline int call(RdmaWorkRequest *req) override{
        msg::Connection* conn = req->conn;
        msg::RdmaConnection* rdma_conn = msg::RdmaStack::rdma_conn_cast(conn);
        if(req->stage == 0){                 //**创建rdma内存，并从底层chunkstore异步读取数据到指定的rdma buffer**//      
            ChunkReadCmd* cmd_chunk_read = new ChunkReadCmd((cmd_t *)req->command);
            cmd_ma_t& ma = ((cmd_chk_io_rd_t *)cmd_chunk_read->get_content())->ma;    
            auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
            
            msg::ib::RdmaBuffer* lbuf = allocator->alloc(cmd_chunk_read->get_ma_len()); //获取一片本地的内存，用于存放数据
            lbuf->data_len = cmd_chunk_read->get_ma_len();
            req->buf_ = lbuf;
            
            chunk_io_rw_mem(cmd_chunk_read->get_chk_id(),cmd_chunk_read->get_off(), cmd_chunk_read->get_ma_len(), (uint64_t)lbuf->buffer(), 0); //read，将数据读到lbuf，然后RDMA_write到rbuf

            req->stage++;
        }else if(req->stage == 1){           //** 新建req进行read/write**//
            ChunkReadCmd* cmd_chunk_read = new ChunkReadCmd((cmd_t *)req->command);
            cmd_ma_t& ma = ((cmd_chk_io_rd_t *)cmd_chunk_read->get_content())->ma; 
            RdmaWorkRequest* rw_req = req->msger_->get_req_pool().alloc_req();

            rw_req->pre_request = req;
            rw_req->service_ = req->service_;
            rw_req->buf_ = req->buf_;
            rw_req->sge_.addr = req->buf_->addr();
            rw_req->sge_.length = req->buf_->size();
            rw_req->sge_.lkey = req->buf_->lkey();
            
            ibv_send_wr &swr = rw_req->send_wr_;
            memset(&swr, 0, sizeof(swr));
            
            swr.wr.rdma.remote_addr = ma.addr;
            swr.wr.rdma.rkey = ma.key;
            swr.wr_id = reinterpret_cast<uint64_t>((msg::RdmaSendWr *)rw_req);
            swr.opcode = IBV_WR_RDMA_WRITE;
            swr.send_flags |= IBV_SEND_SIGNALED;
            swr.num_sge = 1;
            swr.sg_list = &rw_req->sge_;
            swr.next = nullptr;
            
            rw_req->conn = rdma_conn;
            
            rw_req->stage = 2;
            rdma_conn->post_send(rw_req);

            
            // req->stage++;
        }else if(req->stage == 2){         //**发送response**//
            RdmaWorkRequest* pre_req = req->pre_request;

            cmd_rc_t rc = 0;
            cmd_t cmd = *(cmd_t *)pre_req->command;
            ChunkReadCmd* read_cmd = new ChunkReadCmd(&cmd); 
            cmd_res_t* cmd_res = (cmd_res_t *)pre_req->command;
            ChunkReadRes* res = new ChunkReadRes(cmd_res, *read_cmd, rc); 
            pre_req->stage = 3;
            rdma_conn->post_send(pre_req);
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