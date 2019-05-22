/*
 * @Descripttion: 声明并定义四种对chunk的操作函数:read, write, read_zeros, write_zeros
 * @version: 
 * @Author: liweiguang
 * @Date: 2019-05-13 15:07:59
 * @LastEditors: liweiguang
 * @LastEditTime: 2019-05-22 20:26:41
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
#include "libflame/libchunk/log_libchunk.h"

namespace flame {

typedef void(*io_cb_fn_t)(RdmaWorkRequest* req); 

inline void io_cb_func(RdmaWorkRequest* req){
    req->status = RdmaWorkRequest::Status::EXEC_DONE;
    req->run();
    return ;
}
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
inline int chunk_io_rw_mem(chk_id_t chunk_id, chk_off_t offset, uint32_t len, uint64_t laddr, bool rw, io_cb_fn_t cb_fn, RdmaWorkRequest* cb_arg){
    if(rw){ //**write
        char *disk = new char[len];
        memcpy(disk, (void *)laddr, len);
        FlameContext* fct = FlameContext::get_context();
        fct->log()->ltrace("write simdisk down");
    }
    else{   //**read
        char *disk = new char[len];
        strcpy(disk,"abcdefghijklmnopq");
        memcpy((void *)laddr, disk, len);
    }
    cb_fn(cb_arg);
    return RC_SUCCESS;
}


class ReadCmdService final : public CmdService {
public:
    inline int call(RdmaWorkRequest *req) override{
        msg::Connection* conn = req->conn;
        msg::RdmaConnection* rdma_conn = msg::RdmaStack::rdma_conn_cast(conn);
        if(req->status == RdmaWorkRequest::Status::RECV_DONE){                 //**创建rdma内存，并从底层chunkstore异步读取数据到指定的rdma buffer**//      
            ChunkReadCmd* cmd_chunk_read = new ChunkReadCmd((cmd_t *)req->command);
            cmd_ma_t& ma = ((cmd_chk_io_rd_t *)cmd_chunk_read->get_content())->ma;    
            auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
            
            msg::ib::RdmaBuffer* lbuf = allocator->alloc(cmd_chunk_read->get_ma_len()); //获取一片本地的内存，用于存放数据
            lbuf->data_len = cmd_chunk_read->get_ma_len();
            req->data_buf_ = lbuf;
            //read，将数据读到lbuf，io_cb_func的回调在这里实际上是在disk->lbuf后执行req->run()，只是此时req->status = EXEC_DONE
            chunk_io_rw_mem(cmd_chunk_read->get_chk_id(),cmd_chunk_read->get_off(), cmd_chunk_read->get_ma_len(), (uint64_t)lbuf->buffer(), 0, io_cb_func, req); 

        }else if(req->status == RdmaWorkRequest::Status::EXEC_DONE){           //** 进行RDMA WRITE(server write到client相当于读)**//
            ChunkReadCmd* cmd_chunk_read = new ChunkReadCmd((cmd_t *)req->command);
            cmd_ma_t& ma = ((cmd_chk_io_rd_t *)cmd_chunk_read->get_content())->ma; 

            req->sge_.addr = req->data_buf_->addr();
            req->sge_.length = req->data_buf_->size();
            req->sge_.lkey = req->data_buf_->lkey();
            ibv_send_wr &swr = req->send_wr_;
            memset(&swr, 0, sizeof(swr));
            swr.wr.rdma.remote_addr = ma.addr;
            swr.wr.rdma.rkey = ma.key;
            swr.wr_id = reinterpret_cast<uint64_t>((msg::RdmaSendWr *)req);
            swr.opcode = IBV_WR_RDMA_WRITE;
            swr.send_flags |= IBV_SEND_SIGNALED;
            swr.num_sge = 1;
            swr.sg_list = &req->sge_;
            swr.next = nullptr;

            rdma_conn->post_send(req);

        }else if(req->status == RdmaWorkRequest::Status::WRITE_DONE){         //**第二个(rw_req)完成，第一个(request)发送response，因为第一个req保存了头信息，可以直接填充为response发回去**//
            cmd_rc_t rc = 0;
            cmd_t cmd = *(cmd_t *)req->command;
            ChunkReadCmd* read_cmd = new ChunkReadCmd(&cmd); 
            cmd_res_t* cmd_res = (cmd_res_t *)req->command;
            ChunkReadRes* res = new ChunkReadRes(cmd_res, *read_cmd, rc); 
            req->sge_.addr = req->buf_->addr();
            req->sge_.length = 64;
            req->sge_.lkey = req->buf_->lkey();
            req->send_wr_.opcode = IBV_WR_SEND;
            req->send_wr_.sg_list = &req->sge_;
            rdma_conn->post_send(req);
        }else{                              
            return 0;
        }
        return 0;

    }

    ReadCmdService():CmdService(){}
    
    virtual ~ReadCmdService() {}
}; // class ReadCmdService


class WriteCmdService final : public CmdService {
public:
    inline int call(RdmaWorkRequest *req) override{
        // msg::Connection* conn = req->conn;
        // msg::RdmaConnection* rdma_conn = msg::RdmaStack::rdma_conn_cast(conn);
        // if(req->stage == 0){                 //**创建rdma内存，并从远端读取将数据读取到指定的rdma buffer**//    
        //     ChunkWriteCmd* cmd_chunk_write = new ChunkWriteCmd((cmd_t *)req->command);
        //     cmd_ma_t& ma = ((cmd_chk_io_rd_t *)cmd_chunk_write->get_content())->ma; 

        //     auto allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
        //     msg::ib::RdmaBuffer* lbuf = allocator->alloc(cmd_chunk_read->get_ma_len()); //获取一片本地的内存，用于存放数据
        //     lbuf->data_len = cmd_chunk_read->get_ma_len();

        //     RdmaWorkRequest* rw_req = req->msger_->get_req_pool().alloc_req();
        //     rw_req->pre_request = req;
        //     rw_req->service_ = req->service_;
        //     rw_req->buf_ = lbuf;
        //     rw_req->sge_.addr = rw_req->buf_->addr();
        //     rw_req->sge_.length = rw_req->buf_->size();
        //     rw_req->sge_.lkey = rw_req->buf_->lkey();
            
        //     ibv_send_wr &swr = rw_req->send_wr_;
        //     memset(&swr, 0, sizeof(swr));
            
        //     swr.wr.rdma.remote_addr = ma.addr;
        //     swr.wr.rdma.rkey = ma.key;
        //     swr.wr_id = reinterpret_cast<uint64_t>((msg::RdmaSendWr *)rw_req);
        //     swr.opcode = IBV_WR_RDMA_READ;
        //     swr.send_flags |= IBV_SEND_SIGNALED;
        //     swr.num_sge = 1;
        //     swr.sg_list = &rw_req->sge_;
        //     swr.next = nullptr;
            
        //     rw_req->conn = rdma_conn;
        //     rdma_conn->post_send(rw_req);

               
        //     rw_req->stage = 1;
            
            


        //     req->stage++;
        // }else if(req->stage == 1){           //** 新建req进行read/write**//
        //     ChunkWriteCmd* cmd_chunk_read = new ChunkReadCmd((cmd_t *)req->command);
        //     cmd_ma_t& ma = ((cmd_chk_io_rd_t *)cmd_chunk_read->get_content())->ma; 

            
        //     // req->stage++;
        // }else if(req->stage == 2){         //**发送response**//
        //     RdmaWorkRequest* pre_req = req->pre_request;

        //     cmd_rc_t rc = 0;
        //     cmd_t cmd = *(cmd_t *)pre_req->command;
        //     ChunkReadCmd* read_cmd = new ChunkReadCmd(&cmd); 
        //     cmd_res_t* cmd_res = (cmd_res_t *)pre_req->command;
        //     ChunkReadRes* res = new ChunkReadRes(cmd_res, *read_cmd, rc); 
        //     pre_req->stage = 3;
        //     rdma_conn->post_send(pre_req);
        //     req->stage++;
        // }else{                              //**发送完response之后不需要任何操作**//
        //     return 0;
        // }
        return 0;

    }

    WriteCmdService() : CmdService(){}

    virtual ~WriteCmdService() {}
}; // class WriteCmdService


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