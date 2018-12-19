#include "TcpStack.h"
#include "TcpConnection.h"
#include "TcpListenPort.h"

namespace flame{

ListenPort* TcpStack::create_listen_port(NodeAddr *addr){
    return TcpListenPort::create(this->fct, addr);
}

Connection* TcpStack::connect(NodeAddr *addr){
    return TcpConnection::create(this->fct, addr);
}

}