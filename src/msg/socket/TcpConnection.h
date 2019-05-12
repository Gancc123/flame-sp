#ifndef FLAME_MSG_SOCKET_TCP_CONNECTION_H
#define FLAME_MSG_SOCKET_TCP_CONNECTION_H

#include "msg/msg_common.h"
#include "msg/Connection.h"
#include "msg/msg_types.h"
#include "msg/Msg.h"
#include "msg/event/event.h"
#include "common/thread/mutex.h"

#include <atomic>
#include <list>

namespace flame{
namespace msg{

class TcpConnection : public Connection{
    std::list<Msg *> msg_list;
    Mutex msg_list_mutex;
    std::_List_iterator<Msg *> cur_msg;
    MsgBuffer cur_msg_header_buffer;
    size_t cur_msg_offset;
    MsgBufferList::iterator msg_data_iter;

    MsgBuffer recv_header_buffer;
    Msg *cur_recv_msg;
    size_t cur_recv_msg_offset;
    bool cur_msg_is_end() {
        MutexLocker locker(msg_list_mutex);
        return cur_msg == msg_list.end();
    }
    std::atomic<bool> send_msg_posted;
    void append_send_msg(Msg *msg);
    ssize_t submit_cur_msg(bool more);
    static ssize_t do_sendmsg(int fd, struct msghdr &msg, unsigned len, 
                                                                    bool more);
    static int connect(MsgContext *mct, NodeAddr *addr);
    TcpConnection(MsgContext *mct);
public:
    friend class TcpConnEventCallBack;
    static TcpConnection* create(MsgContext *mct, int fd);
    static TcpConnection* create(MsgContext *mct, NodeAddr *addr);
    ~TcpConnection();

    virtual msg_ttype_t get_ttype() override { return msg_ttype_t::TCP; }
    
    virtual ssize_t send_msg(Msg *msg, bool more=false) override;
    virtual Msg* recv_msg() override;
    virtual int pending_msg() override {
        MutexLocker locker(msg_list_mutex);
        return this->msg_list.size();
    };

    virtual bool is_connected() override;
    virtual void close() override;
    enum class WriteStatus {
        NOWRITE,
        CANWRITE,
        CLOSED
    };
    std::atomic<WriteStatus> can_write;

    virtual void read_cb() override;
    virtual void write_cb() override;
    virtual void error_cb() override;
    
};

} //namespace msg
} //namespace flame

#endif