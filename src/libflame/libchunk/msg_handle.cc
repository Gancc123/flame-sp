/*
 * @Descripttion: 
 * @version: 
 * @Author: liweiguang
 * @Date: 2019-05-16 14:56:17
 * @LastEditors: liweiguang
 * @LastEditTime: 2019-05-20 10:01:24
 */
#include "libflame/libchunk/msg_handle.h"

#include "include/csdc.h"
#include "util/clog.h"
#include "include/retcode.h"
#include "libflame/libchunk/log_libchunk.h"
#include "include/cmd.h"
#include "libflame/libchunk/chunk_cmd_service.h"

namespace flame {

//--------------------------RdmaWorkRequest------------------------------------------------------
/**
 * @name: create_request
 * @describtions: 创建44种不同的request
 * @param   msg::MsgContext*        msg_context         Msg上下文
 *          Msger*                  msger               先基本功能只是用来recv_request和区分服务器/客户端
 * @return: RdmaWorkRequest*
 */
RdmaWorkRequest* RdmaWorkRequest::create_request(msg::MsgContext* msg_context, Msger* msger){
    RdmaWorkRequest* req = new RdmaWorkRequest(msg_context, msger);
    if(!req){
        return nullptr;
    }
    RdmaBuffer* buffer = msg::Stack::get_rdma_stack()->get_rdma_allocator()->alloc(4096);//4KB这里的buffer会自己释放吗？
    if(!buffer){
        delete req;
        return nullptr;
    }
    req->buf_ = buffer;
    req->sge_.addr = buffer->addr();
    req->sge_.length = 64;
    req->sge_.lkey = buffer->lkey();

    ibv_send_wr &swr = req->send_wr_;
    memset(&swr, 0, sizeof(swr));
    swr.wr_id = reinterpret_cast<uint64_t>((RdmaSendWr *)req);
    swr.opcode = IBV_WR_SEND;
    swr.send_flags |= IBV_SEND_SIGNALED;
    swr.num_sge = 1;
    swr.sg_list = &req->sge_;

    ibv_recv_wr &rwr = req->recv_wr_;
    memset(&rwr, 0, sizeof(rwr));
    rwr.wr_id = reinterpret_cast<uint64_t>((RdmaRecvWr *)req);
    rwr.num_sge = 1;
    rwr.sg_list = &req->sge_;

    req->command = req->buf_->buffer();
    return req;
}

// /**
//  * @name: transform_rw_request
//  * @describtions: 根据用户已经填充好的Command进行Work Request类型转换，主要是为server端提供服务，因为client端不会outbound read/write
//  * @param  
//  * @return: 
//  */
// void RdmaWorkRequest::transform_rw_request(){
//     /*这里缺少正确性检查，不知道是否设置了Command**/
//     uint16_t cn = command->get_num();   
//     cmd_ma_t* ma = ((cmd_chk_io_rd_t*)command->get_content())->ma;

//     RdmaBuffer* buffer = msg::Stack::get_rdma_stack()->get_rdma_mallocator()->alloc(4096);//4KB这里的buffer会自己释放吗？
//     if(!buffer){
//         return nullptr;
//     }
//     buf_ = buffer;
//     sge_.addr = buffer->addr();
//     sge_.length = ma->len;
//     sge_.lkey = buffer->lkey();

//     send_wr_.rdma.remote_addr = ma->addr;
//     send_wr_.rdma.rkey = ma->key;

//     memset(&send_wr_, 0, sizeof(send_wr_));
//     send_wr_.wr_id = reinterpret_cast<uint64_t>((RdmaSendWr *)this);
//     send_wr_.num_sge = 1;
//     send_wr_.sg_list = &sge_;
    
//     /*这里不太确定低8位还是高8位是seq，这里先假设是低8位*/
//     if((cn & 0xff) == CMD_CHK_IO_READ){
//         ChunkReadCmd* cmd_chunk_read = (ChunkReadCmd *)command;
//         chunk_io_rw_mem(cmd_chunk_read->get_chk_id(),cmd_chunk_read->get_off(), cmd_chunk_read->get_ma_len(), sge_.addr, 0); //read，将数据读到lbuf，然后RDMA_write到rbuf 这里好像阻塞了？？？先这样吧
//         send_wr_.opcode = IBV_WR_RDMA_WRITE;
//     }else{
//         send_wr_.opcode = IBV_WR_RDMA_READ;
//     } 
    
//     return ;
// }

// /**
//  * @name: transform_res
//  * @describtions: 填充response然后发送给客户端
//  * @param  
//  * @return: 
//  */
// void RdmaWorkRequest::transform_res(){
//     swr.opcode = IBV_WR_SEND;
//     swr.send_flags |= IBV_SEND_SIGNALED;
//     cmd_rc_t rc = 0;
//     command = new (command) ChunkReadRes(*command, rc);
//     return ;
// }


RdmaWorkRequest::~RdmaWorkRequest(){
    if(buf_){
        msg::Stack::get_rdma_stack()->get_rdma_allocator()->free(buf_);
        buf_ = nullptr;
    }
}

//**以下四个函数都是状态机的变化，最终会调用run()来执行实际的操作
/**
 * @name: on_send_cancelled
 * @describtions: 
 * @param 
 * @return: 
 */
void RdmaWorkRequest::on_send_cancelled(bool err, int eno){
    status = DESTROY;
    run();
}

void RdmaWorkRequest::on_recv_cancelled(bool err, int eno){
    status = DESTROY;
    run();
}

/**
 * @name: on_send_done
 * @describtions: RDMA发送成功后的回调
 * @param   ibv_wc&             cqe         completion queue element判断是否成功
 * @return: 
 */
void RdmaWorkRequest::on_send_done(ibv_wc &cqe){
    if(cqe.status == IBV_WC_SUCCESS){
        status = SEND_DONE;
    }else {
        status = ERROR;
    }
    run();
}

/**
 * @name: on_recv_done
 * @describtions: 接收到WorkRequest的回调
 * @param   RdmaConnection*     conn        对端的连接
 *          ibv_wc&             cqe         完成元素，接受是否成功            
 * @return: 
 */
void RdmaWorkRequest::on_recv_done(msg::RdmaConnection *conn, ibv_wc &cqe){
    this->conn = conn;
    ML(msg_context_, info, "I recv !!");
    if(cqe.status == IBV_WC_SUCCESS){
        status = RECV_DONE;
    }else{
        status = ERROR;
    }
    run();
}

/**
 * @name: run
 * @describtions: 状态机变化的真正执行函数，包括五种状态
 * @param 
 * @return: 
 */
void RdmaWorkRequest::run(){
    bool next_ready;
    if(msger_->is_server()){
    CmdServiceMapper* cmd_service_mapper = CmdServiceMapper::get_cmd_service_mapper();
    ML(msg_context_, info, "I recv !!");
    FlameContext* flame_context = FlameContext::get_context();
    flame_context->log()->ldebug("aaaaa  %u", ((cmd_t *)command)->hdr.len);
    flame_context->log()->ldebug("aaaaa  %u", ((cmd_t *)command)->hdr.cn.cls);
    flame_context->log()->ldebug("aaaaa  %u", ((cmd_t *)command)->hdr.cn.seq);

    CmdService* service = cmd_service_mapper->get_service(((cmd_t *)command)->hdr.cn.cls, ((cmd_t *)command)->hdr.cn.seq);
    ML(msg_context_, info, "I recv !!");
        do{
            next_ready = false;
            switch(status){
                case RECV_DONE:
                    service->call(this);                    //*stage 0
                    status = EXEC_DONE;
                    next_ready = true;
                    break;
                case EXEC_DONE:
                    assert(conn);
                    service->call(this);                    //*stage 1
                    status = DESTROY;
                    next_ready = true;
                    break;
                case SEND_DONE:         //**如果是read/write，SEND_DONE是否代表read/write已经成功????????????????????????? **//
                    service->call(this);                    //**stage 2 注意stage 2是在令一个request中
                    status = DESTROY;
                    next_ready = true;
                    break;
                case DESTROY:
                case ERROR:
                    msger_->get_req_pool().free_req(this);
                    status = FREE;
                    break;
            };
        }while(next_ready);
    }else{
        // do{
        //     next_ready = false;
        //     switch(status){
        //     case RECV_DONE:
        //         ML(mct, info, "Recv from server: {}", data->count);
        //         if(data->count >= 200){
        //             clog("client iter done.");
        //             status = DESTROY;
        //             next_ready = true;
        //             break;
        //         }
        //         assert(conn);
        //         conn->post_send(this);
        //         break;
        //     case SEND_DONE:
        //         status = DESTROY;
        //         next_ready = true;
        //         break;
        //     case DESTROY:
        //     case ERROR:
        //         msger->get_req_pool().free_req(this);
        //         status = FREE;
        //         break;
        //     };
        // }while(next_ready);
    }
}

//-------------------------------------------RdmaWorkRequestPool------------------------------------//
RdmaWorkRequestPool::RdmaWorkRequestPool(msg::MsgContext *c, Msger *m)
    :msg_context_(c), msger_(m), mutex_(MUTEX_TYPE_ADAPTIVE_NP){

}

RdmaWorkRequestPool::~RdmaWorkRequestPool(){
}

/**
 * @name: expand_lockfree
 * @describtions: 创建RdmaWOrkRequest到reqs_free_待用
 * @param   int     n       创建的req数量
 * @return: 成功建立的req数量
 */
int RdmaWorkRequestPool::expand_lockfree(int n){
    int i;
    for(i = 0; i < n; i++){
        RdmaWorkRequest* req = RdmaWorkRequest::create_request(msg_context_, msger_);
        if(!req) break;
        reqs_free_.push_back(req);
    }
    return i;
}

/**
 * @name: purge_lockfree
 * @describtions: 释放申请的req，这些req需要在req_free_上
 * @param   int     n      指定释放的req数量，-1表示全释放
 * @return: 释放的req数量
 */
int RdmaWorkRequestPool::purge_lockfree(int n){
    int cnt = 0;
    while(!reqs_free_.empty() && cnt != n){
        RdmaWorkRequest* req = reqs_free_.back();
        delete req;
        reqs_free_.pop_back();
        cnt++;
    }
    return cnt;
}

/**
 * @name: alloc_reqs
 * @describtions: 从reqs_free_中取出一批reqs，如果不够则通过expand_lockfree进行创建
 * @param   vector<RdmaWorkRequest *>&      reqs        将取出的这批req放入传入的引用reqs中
 * @return: 采集的req数量
 */
int RdmaWorkRequestPool::alloc_reqs(int n, std::vector<RdmaWorkRequest *> &reqs){
    MutexLocker l(mutex_);
    if(n > reqs_free_.size()){
        expand_lockfree(n - reqs_free_.size());
    }
    if(n > reqs_free_.size()){
        n = reqs_free_.size();
    }
    reqs.insert(reqs.begin(), reqs_free_.end() - n, reqs_free_.end());
    reqs_free_.erase(reqs_free_.end() - n, reqs_free_.end());
    return n;
}

/**
 * @name: alloc_req
 * @describtions: 从reqs_free_中采集req，reqs_free_为空则create_request
 * @param 
 * @return: RdmaWorkRequest*    采集的一个RdmaWorkRequest指针 
 */
RdmaWorkRequest* RdmaWorkRequestPool::alloc_req(){
    MutexLocker l(mutex_);
    if(reqs_free_.empty()){
        if(expand_lockfree(1) <= 0){
            return nullptr;
        }
    }
    RdmaWorkRequest* req = reqs_free_.back();
    reqs_free_.pop_back();
    return req;
}

/**
 * @name: free_req
 * @describtions: 内部的释放是将req放入reqs_free_供循环利用
 * @param   RdmaWorkRequest*        req         放回reqs_free_的req指针
 * @return: 
 */
void RdmaWorkRequestPool::free_req(RdmaWorkRequest* req){
    MutexLocker l(mutex_);
    reqs_free_.push_back(req);
}
/**
 * @name: purge
 * @describtions: public函数，为用户提供释放req的接口
 * @param   int         n           释放的req个数，注意这里的purge就是真正的delete
 * @return: int 真正delete的req数量
 */
int RdmaWorkRequestPool::purge(int n){
    MutexLocker l(mutex_);
    return purge_lockfree(n);
}


//-------------------------------------------------Msger-------------------------------------------//
/**
 * @name: get_recv_wrs
 * @describtions: 提供给msg模块进行recv work request的填充
 * @param   int                     n           recv work request数量
 *          vector<RdmaRecvWr *>&   wrs         引用，获得n个RdmaRecvWr
 * @return: 获得的recv work request数量
 */
int Msger::get_recv_wrs(int n, std::vector<msg::RdmaRecvWr *> &wrs){
    std::vector<RdmaWorkRequest *> reqs;
    int rn = pool_.alloc_reqs(n, reqs);
    wrs.reserve(reqs.size());
    for(auto r : reqs){
        wrs.push_back((msg::RdmaRecvWr *)r);
    }
    return rn;
}

/**
 * @name: on_conn_declared
 * @describtions: 在将要建立连接时的回调
 * @param   Connection*     conn        
 *          Session*        s
 * @return: 
 */
void Msger::on_conn_declared(msg::Connection *conn, msg::Session *s){
    ML(msg_context_, info, "Session: {}", s->to_string());
}

/**
 * @name: on_conn_recv
 * @describtions: 接收到消息的回调
 * @param {type} 
 * @return: 
 */
void Msger::on_conn_recv(msg::Connection *conn, msg::Msg *msg){
    ML(msg_context_, info, "I recv something.");
};

} //namespace flame