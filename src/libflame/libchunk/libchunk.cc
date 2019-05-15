#include "libflame/libchunk/libchunk.h"

#include "msg/msg_core.h"
#include "libflame/libchunk/msg_recv.h"


namespace flame {

//-------------------------------------MemoryAreaImpl->MemoryArea-------------------------------------------------------//
MemoryAreaImpl::MemoryAreaImpl(uint64_t addr, uint32_t len, uint32_t key, bool is_dma)
            : MemoryArea(), is_dma_(is_dma){
    ma_.addr = addr;
    ma_.len = len;
    ma_.key = key; 
}

bool MemoryAreaImpl::is_dma() const {
    return is_dma_;
}
void* MemoryAreaImpl::get_addr() const {
    return (void *)ma_.addr;
}
uint64_t MemoryAreaImpl::get_addr_uint64() const {
    return ma_.addr;
}
uint32_t MemoryAreaImpl::get_len() const {
    return ma_.len;
}
uint32_t MemoryAreaImpl::get_key() const {
    return ma_.key;
}
cmd_ma_t MemoryAreaImpl::get() const {
    return ma_;
}   



//-------------------------------------CmdClientStubImpl->CmdClientStub-------------------------------------------------//
/**
 * @name: CmdClientStubImpl
 * @describtions: CmdClientStubImpl构造函数，通过FlameContext*构造MsgContext*
 * @param   FlameContext*       flame_context 
 * @return: \
 */
CmdClientStubImpl::CmdClientStubImpl(FlameContext *flame_context)
    :  CmdClientStub(){
    msg_context_ = new msg::MsgContext(flame_context); //* set msg_context_
    msger_client_cb_ = new MsgerClientCallback(msg_context_);
    msg_context_->init(msger_client_cb_, nullptr);//* set msg_client_recv_func
}

/**
 * @name: _set_session
 * @describtions: CmdClientStubImpl设置session_
 * @param   std::string     ip_addr     字符串形式的IP地址       
 *          int             port        端口号 
 * @return: 0表示成功
 */
int CmdClientStubImpl::_set_session(std::string ip_addr, int port){
    /**此处的NodeAddr仅作为msger_id_t唯一标识的参数，并不是实际通信地址的填充**/
    msg::NodeAddr* addr = new msg::NodeAddr(msg_context_);
    addr->ip_from_string(ip_addr);
    addr->set_port(port);
    msg::msger_id_t msger_id = msg::msger_id_from_msg_node_addr(addr);
    addr->put();
    session_ = msg_context_->manager->get_session(msger_id);
    /*此处填充通信地址*/
    msg::NodeAddr* rdma_addr = new msg::NodeAddr(msg_context_);
    rdma_addr->set_ttype(NODE_ADDR_TTYPE_RDMA);
    rdma_addr->ip_from_string(ip_addr);
    rdma_addr->set_port(port);
    session_->set_listen_addr(rdma_addr, msg::msg_ttype_t::RDMA);
    rdma_addr->put();
    return 0;
}

/**
 * @name: CmdeClientStub
 * @describtions: 创建访问CSD的客户端句柄
 * @param   std::string     ip_addr     字符串形式的IP地址       
 *          int             port        端口号
 * @return: std::shared_ptr<CmdClientStubImpl>
 */
std::shared_ptr<CmdClientStub> CmdClientStubImpl::create_stub(std::string ip_addr, int port){
    FlameContext* flame_context = FlameContext::get_context();
    CmdClientStubImpl* cmd_client_stub = new CmdClientStubImpl(flame_context);
    int rc;
    rc = cmd_client_stub->_set_session(ip_addr, port);
    if(rc) return nullptr;
    return std::shared_ptr<CmdClientStub>(cmd_client_stub);
} 

/**
 * @name: submit 
 * @describtions: libflame端提交请求到CSD端 
 * @param   Conmmand&           cmd         准备发送的命令
 *          cmd_cb_fn_t*        cb_fn       命令得到回复后的回调函数
 *          void*               cb_arg      回调函数的参数
 * @return: 
 */
int CmdClientStubImpl::submit(Command& cmd, cmd_cb_fn_t cb_fn, void* cb_arg){
    MsgCmd msg_request_cmd;
    cmd.copy(&msg_request_cmd.cmd);
    msg::Connection* conn = session_->get_conn(msg::msg_ttype_t::RDMA);
    auto req_msg = msg::Msg::alloc_msg(msg_context_);
    req_msg->append_data(msg_request_cmd);
    conn->send_msg(req_msg);
    req_msg->put();
    struct MsgCallBack msg_cb;
    msg_cb.cb_fn = cb_fn;
    msg_cb.cb_arg = cb_arg;
    msg_cb_q_.push(msg_cb);
    
    return 0;
}


//-------------------------------------CmdServerStubImpl->CmdServerStub-------------------------------------------------//
CmdServerStubImpl::CmdServerStubImpl(FlameContext* flame_context){
    msg_context_ = new msg::MsgContext(flame_context); //* set msg_context_
    msger_server_cb_ = new MsgerServerCallback(msg_context_);
    msg_context_->init(msger_server_cb_, nullptr);//* set msg_server_recv_func
}

} // namespace flame
