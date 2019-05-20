#ifndef FLAME_LIBFLAME_LIBCHUNK_MSG_HANDLE_H
#define FLAME_LIBFLAME_LIBCHUNK_MSG_HANDLE_H

#include "msg/msg_core.h"
#include "include/cmd.h"
#include "common/thread/mutex.h"
// #include "libflame/libchunk/chunk_cmd_service.h"

#include <deque>
#include <sys/queue.h>

namespace flame {

class RequestPool;
class Msger;

//----------------RdmaWorkRequest----------------------------//
class RdmaWorkRequest : public msg::RdmaRecvWr, public msg::RdmaSendWr{
public:
    friend class ReadCmdService;
    enum Status{
        FREE = 0,
        RECV_DONE,
        EXEC_DONE,
        SEND_DONE,
        DESTROY,
        ERROR,
    };
private:
    using RdmaBuffer = msg::ib::RdmaBuffer;
    msg::MsgContext *msg_context_;
    Msger *msger_;
    ibv_sge sge_;
    ibv_send_wr send_wr_;
    ibv_recv_wr recv_wr_;
    RdmaBuffer *buf_;
    RdmaWorkRequest(msg::MsgContext *c, Msger *m)
    : msg_context_(c), msger_(m), status(FREE), conn(nullptr), stage(0) {}
public:
    Status status;
    msg::RdmaConnection *conn;
    void *command;
    int stage;

    static RdmaWorkRequest* create_request(msg::MsgContext *c, Msger *m);//创建默认的send/recv request，如果要write or read，则需要额外设置

    // void transform_rw_request(); //根据command来进行转换

    ~RdmaWorkRequest();

    inline virtual ibv_send_wr *get_ibv_send_wr() override{
        return &send_wr_;
    }

    inline virtual ibv_recv_wr *get_ibv_recv_wr() override{
        return &recv_wr_;
    }

    virtual void on_send_done(ibv_wc &cqe) override;

    virtual void on_send_cancelled(bool err, int eno=0) override;

    virtual void on_recv_done(msg::RdmaConnection *conn, ibv_wc &cqe) override;

    virtual void on_recv_cancelled(bool err, int eno=0) override;

    void run();

};//class RdmaWorkRequest


//--------------------------RdmaWorkRequestPool-------------------------------------------------//
class RdmaWorkRequestPool{
    msg::MsgContext *msg_context_;
    Msger *msger_;
    std::deque<RdmaWorkRequest *> reqs_free_;
    Mutex mutex_;
    int expand_lockfree(int n);
    int purge_lockfree(int n);
public:
    explicit RdmaWorkRequestPool(msg::MsgContext *c, Msger *m);
    ~RdmaWorkRequestPool();

    int alloc_reqs(int n, std::vector<RdmaWorkRequest *> &reqs);
    RdmaWorkRequest* alloc_req();
    void free_req(RdmaWorkRequest *req);

    // int expand(int n);
    int purge(int n);
};//class RdmaWorkRequestPool


//--------------------------Msger在msg v2中几乎没什么用，回调已放到RdmaWorkRequest中-----------------------------------------------------//

class Msger : public msg::MsgerCallback{
    msg::MsgContext *msg_context_;
    RdmaWorkRequestPool pool_;
    bool is_server_;
public:
    explicit Msger(msg::MsgContext *c, bool s) 
    : msg_context_(c), pool_(c, this), is_server_(s) {};
    virtual void on_conn_declared(msg::Connection *conn, msg::Session *s) override;
    virtual void on_conn_recv(msg::Connection *conn, msg::Msg *msg) override ;
    virtual int get_recv_wrs(int n, std::vector<msg::RdmaRecvWr *> &wrs) override;

    RdmaWorkRequestPool &get_req_pool() { return pool_; }
    bool is_server() { return is_server_; }
};//class Msger

} //namespace flame


#endif //FLAME_LIBFLAME_LIBCHUNK_MSG_HANDLE_H