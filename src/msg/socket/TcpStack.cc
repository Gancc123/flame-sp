#include "TcpStack.h"
#include "TcpConnection.h"
#include "TcpListenPort.h"

namespace flame{
namespace msg{

ListenPort* TcpStack::create_listen_port(NodeAddr *addr){
    return TcpListenPort::create(this->mct, addr);
}

Connection* TcpStack::connect(NodeAddr *addr){
    return TcpConnection::create(this->mct, addr);
}

} //namespace msg
} //namespace flame