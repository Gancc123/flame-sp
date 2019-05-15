#ifndef FLAME_LIBFLAME_LIBCHUNK_MSG_RECV_H
#define FLAME_LIBFLAME_LIBCHUNK_MSG_RECV_H

#include "msg/msg_core.h"
#include "include/cmd.h"
#include "include/csdc.h"
#include "libflame/libchunk/log_libchunk.h"

#include <queue>
#include <map>

namespace flame {

struct MsgCmd : public msg::MsgData{
    cmd_t cmd;
    virtual int encode(msg::MsgBufferList &bl) override;
    virtual int decode(msg::MsgBufferList::iterator &it) override;
};//class MsgCmd

//--------------------------Client-----------------------------------------------------

class MsgerClientCallback : public msg::MsgerCallback{
    msg::MsgContext *mct_;
public:
    explicit MsgerClientCallback(msg::MsgContext *c) : mct_(c) {}
    virtual void on_conn_recv(msg::Connection *conn, msg::Msg *msg) override;
}; //class MsgerClientCallback


//--------------------------Server-----------------------------------------------------//

class MsgerServerCallback : public msg::MsgerCallback{
    msg::MsgContext *mct_;
public:
    explicit MsgerServerCallback(msg::MsgContext *c) : mct_(c) {};
    virtual void on_conn_recv(msg::Connection *conn, msg::Msg *msg) override;
};//class MsgerServerCallback

} //namespace flame


#endif //FLAME_LIBFLAME_LIBCHUNK_MSG_RECV_H