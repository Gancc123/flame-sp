#ifndef FLAME_LIBFLAME_LIBCHUNK_LIBCHUNK_H
#define FLAME_LIBFLAME_LIBCHUNK_LIBCHUNK_H

#include "include/cmd.h"

#include "msg/msg_core.h"
#include "libflame/libchunk/msg_recv.h"


namespace flame {

//------------------------------------MemoryAreaImpl->MemoryArea--------------------------------------------------------//
class MemoryAreaImpl : public MemoryArea{
public:
    MemoryAreaImpl(uint64_t addr, uint32_t len, uint32_t key, bool is_dma);
    
    ~MemoryAreaImpl(){}

    virtual bool is_dma() const override;
    virtual void *get_addr() const override;
    virtual uint64_t get_addr_uint64() const override;
    virtual uint32_t get_len() const override;
    virtual uint32_t get_key() const override;
    virtual cmd_ma_t get() const override;

private:
    cmd_ma_t ma_;  //* including addr, len, key
    bool is_dma_;
};


//-------------------------------------CmdClientStubImpl->CmdClientStub-------------------------------------------------//
struct MsgCallBack{
    cmd_cb_fn_t cb_fn;
    void* cb_arg;
};

class CmdClientStubImpl : public CmdClientStub{
public:
    static std::shared_ptr<CmdClientStub> create_stub(std::string ip_addr, int port);
    
    virtual int submit(Command& cmd, cmd_cb_fn_t cb_fn, void* cb_arg) override;

    CmdClientStubImpl(FlameContext* flame_context);

    ~CmdClientStubImpl() {
        msg_context_->fin();
        delete msg_context_;
    }

private:

    int _set_session(std::string ip_addr, int port);

    msg::MsgContext* msg_context_;
    MsgerClientCallback* msger_client_cb_;
    msg::Session* session_;
    std::queue<MsgCallBack> msg_cb_q_;

}; // class CmdClientStubImpl


class CmdServerStubImpl : public CmdServerStub{
public:

    CmdServerStubImpl(FlameContext* flame_context);
    virtual ~CmdServerStubImpl() {
        msg_context_->fin();
        delete msger_server_cb_;
        delete msg_context_;
    }

private:
    msg::MsgContext* msg_context_;
    MsgerServerCallback* msger_server_cb_;
    // msg::Session* session_; //on_listen_accept()中会有但是如何传出来？

}; // class CmdServerStubImpl


} // namespace flame

#endif //FLAME_LIBFLAME_LIBCHUNK_LIBCHUNK_H