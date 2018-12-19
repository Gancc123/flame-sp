#include "Msg.h"
#include "internal/byteorder.h"
#include "msg_data.h"

#include <netinet/in.h>

namespace flame{

Msg::~Msg(){
    
}

ssize_t Msg::decode_header(MsgBuffer &buffer){
    if(buffer.offset() < FLAME_MSG_HEADER_LEN){
        return -1;
    }

    flame_msg_header_t *header = (flame_msg_header_t *)buffer.data();

    type = header->type;

    flag = header->flag;

    priority = header->priority;

    reserved = header->reserved;

    data_len = header->len;

    return FLAME_MSG_HEADER_LEN;
}


ssize_t Msg::encode_header(MsgBuffer &buffer){
    if(buffer.length() < FLAME_MSG_HEADER_LEN){
        return -1;
    }
    flame_msg_header_t *header = (flame_msg_header_t *)buffer.data();

    header->type = type;

    header->flag = flag;

    header->priority = priority;

    header->reserved = reserved;

    header->len = get_data_len();

    buffer.set_offset(FLAME_MSG_HEADER_LEN);



    return FLAME_MSG_HEADER_LEN;
}


}

