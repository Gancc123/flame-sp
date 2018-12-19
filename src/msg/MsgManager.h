#ifndef FLAME_MSG_MSG_MANAGER_H
#define FLAME_MSG_MSG_MANAGER_H

#include "internal/node_addr.h"
#include "internal/types.h"
#include "msg_types.h"
#include "Session.h"
#include "MsgWorker.h"

#include <vector>
#include <set>
#include <map>

namespace flame{

class MsgManager : public ListenPortListener, public ConnectionListener{
    FlameContext *fct;
    std::map<NodeAddr *, ListenPort *> listen_map;
    std::map<msger_id_t, Session *, msger_id_comparator> session_map;
    std::vector<MsgWorker *> workers;
    MsgerCallback *m_msger_cb;
    std::atomic<bool> is_running;

    std::set<Connection *> session_unknown_conns;
    Msg *declare_msg = nullptr;

    Mutex m_mutex;

public:
    explicit MsgManager(FlameContext *c, int worker_num=4)
    :fct(c), is_running(false), m_mutex(MUTEX_TYPE_ADAPTIVE_NP){
        workers.reserve(worker_num);
        for(int i = 0;i < worker_num; ++i){
            workers.push_back(new MsgWorker(fct, i));
        }
    }

    ~MsgManager();

    void set_msger_cb(MsgerCallback *msger_cb){
        this->m_msger_cb = msger_cb;
    }

    MsgerCallback *get_msger_cb() const{
        return this->m_msger_cb;
    }

    int get_worker_num() const {
        return workers.size();
    }

    MsgWorker *get_worker(int index){
        if(index < 0 || index >= workers.size()){
            return nullptr;
        }
        return workers[index];
    }

    MsgWorker *get_lightest_load_worker();

    ListenPort *add_listen_port(NodeAddr *addr, msg_ttype_t ttype);
    int del_listen_port(NodeAddr *addr);

    Connection* add_connection(NodeAddr *addr, msg_ttype_t ttype);
    int del_connection(Connection* conn);

    Session *get_session(msger_id_t msger_id);
    int del_session(msger_id_t msger_id);

    int start();
    int clear_before_stop();
    int stop();
    bool running() const { return this->is_running; }

    virtual void on_listen_accept(ListenPort *lp, Connection *conn) override;
    virtual void on_listen_error(NodeAddr *listen_addr) override;

    virtual void on_conn_recv(Connection *, Msg *) override;
    virtual void on_conn_error(Connection *conn) override;

private:
    Msg *get_declare_msg();

    void add_lp(ListenPort *lp);
    void del_lp(ListenPort *lp);
    void add_conn(Connection *conn);
    void del_conn(Connection *conn);
};


}

#endif