#include "Connection.h"
#include "Session.h"

namespace flame{

Connection::~Connection(){
}

Session* Connection::get_session() const{
    return this->session; //* 这里获得的session可能已被释放
}

void Connection::set_session(Session *s){
    this->session = s;  //* 这里s->get()会造成循环引用
}

}