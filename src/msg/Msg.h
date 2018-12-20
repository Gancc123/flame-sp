#ifndef FLAME_MSG_MESSAGE_H
#define FLAME_MSG_MESSAGE_H

#include <list>
#include <vector>
#include <utility>
#include <stdexcept>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "msg_common.h"

namespace flame{

class Msg : public RefCountedObject{
    MsgBufferList data_bl;

public:
    static Msg* alloc_msg(FlameContext *fct, 
                                    msg_ttype_t ttype=msg_ttype_t::TCP){
        Msg *msg = new Msg(fct);
        msg->ttype = ttype;
        msg->type = FLAME_MSG_TYPE_CTL;
        return msg;
    }

    explicit Msg(FlameContext *fct)
    : RefCountedObject(fct){
        type = FLAME_MSG_TYPE_NONE;
        priority = FLAME_MSG_PRIO_DEFAULT;
    };

    ~Msg() {};

    ssize_t decode_header(MsgBuffer &buffer);
    ssize_t encode_header(MsgBuffer &buffer);

    void set_data(MsgBufferList &&data){
        data_bl.swap(data);
    }

    int append_data(MsgBuffer &buf){
        return data_bl.append(buf);
    }

    int append_data(void *buf, size_t len){
        return  data_bl.append(buf, len);
    }

    int append_data(MsgData &d){
        return d.encode(data_bl);
    }

    MsgBufferList::iterator data_iter(){
        return data_bl.begin();
    }

    MsgBufferList& data_buffer_list(){
        return data_bl;
    }

    // !!not use data_len field to get the real data len
    // data_len is only for recv_msg to ensure the data followed.
    // use get_data_len() instead.
    size_t get_data_len() const{
        return data_bl.length();
    }

    size_t total_bytes() const{
        return get_data_len() + sizeof(flame_msg_header_t);
    }

    int clear_data(){
        data_bl.clear();
    }

    char *cur_data_buffer(int &cd_len){
        int cd_len_bl = 0;
        char *buffer = data_bl.cur_data_buffer(cd_len_bl, 
                                                data_len - data_bl.length());
        cd_len = std::min((int)(data_len - data_bl.length()), cd_len_bl);
        return buffer;
    }

    int cur_data_buffer_extend(int ex_len){
        if(ex_len > data_len - data_bl.length()){
            ex_len = data_len - data_bl.length();
        }
        int r =  data_bl.cur_data_buffer_extend(ex_len);
        return r;
    }

    bool is_ctl() const {
        return (this->type == FLAME_MSG_TYPE_CTL);
    }

    bool is_io() const {
        return (this->type == FLAME_MSG_TYPE_IO);
    }

    bool is_declare_id() const{
        return (this->type == FLAME_MSG_TYPE_DECLARE_ID);
    }

    bool is_nop() const {
        return (this->type == FLAME_MSG_TYPE_NONE);
    }

    void set_flags(uint8_t flags, bool remove=false) {
        if(!remove){
            this->flag |= flags;
        }else{
            this->flag &= (~flags);
        }
    }

    bool is_req() const{
        return !is_resp();
    }

    bool is_resp() const {
        return (this->flag & FLAME_MSG_FLAG_RESP);
    }

    bool has_rdma() const {
        return (this->flag & FLAME_MSG_FLAG_RDMA);
    }

    uint8_t get_rdma_type() const{
        return FLAME_MSG_RDMA_TYPE(flag);
    }

    uint8_t get_rdma_cnt() const{
        return FLAME_MSG_RDMA_CNT(reserved);
    }

    void set_rdma_type(uint8_t t){
        set_flags(FLAME_MSG_FLAG_MEM_FETCH, t != FLAME_MSG_RDMA_MEM_FETCH);
    }

    void set_rdma_cnt(uint8_t cnt){
        reserved = cnt;
    }

    std::string msg_type_str() const{
        switch(this->type){
        case FLAME_MSG_TYPE_NONE:
            return "TYPE_NONE";
        case FLAME_MSG_TYPE_CTL:
            return "TYPE_CTL";
        case FLAME_MSG_TYPE_IO:
            return "TYPE_IO";
        case FLAME_MSG_TYPE_DECLARE_ID:
            return "TYPE_DECLARE_ID";
        default:
            return "TYPE_INVALID";
        }
    }

    std::string msg_flag_str() const{
        char flags[4];
        flags[sizeof(flags) - 1] = '\0';
        flags[2] = has_rdma()?'M':'-';
        flags[1] = is_resp()?'R':'-';
        flags[0] = '-';
        return flags;
    }

    std::string to_string() const{
        std::stringstream ss;
        ss << "[Msg " << msg_ttype_to_str(ttype) 
                    << " " << msg_type_str()
                    << " " << msg_flag_str()
                    << " SL" << (int)priority 
                    << " " << get_data_len() << "B](" << (void *)this << ")";
        return ss.str();
    }

    msg_ttype_t ttype = msg_ttype_t::TCP; //just for transport type

    uint8_t  type; // msg type
    uint8_t  flag = 0; // msg flag
    uint8_t  priority; 
    uint8_t  reserved = 0;
    uint32_t data_len = 0; // only for receive

    size_t payload_len() const{
        if(has_rdma()){
            return get_rdma_cnt() * sizeof(flame_msg_rdma_mr_t);
        }
        return 0;
    }

};

}

#endif //FLAME_MSG_MESSAGE_H
