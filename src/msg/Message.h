#ifndef FLAME_MSG_MESSAGE_H
#define FLAME_MSG_MESSAGE_H

#include "acconfig.h"
#include "internal/byteorder.h"

#include "util/fmt.h"

#include <memory>

#define MESSAGE_RESERVED_LEN 32
#define MESSAGE_COMPID_BITLEN 48
#define MESSAGE_COMPID_MASK 0xFFFFFFFFFFFFULL

namespace flame {
namespace msg{

struct message_header_t {
    // component addr
    // [63, 48] type: MGR/CSD/GW
    // [47, 0] id
    uint64_t src;
    uint64_t dst;

    // request/respond number
    uint8_t typ; // admin or io
    uint8_t tgt; // target of request, eg. MGR/CSD/GW
    uint8_t cls; // class of request, eg. volume/chunk, used to group requests
    uint8_t num; // number of request

    uint16_t rqg; // request group
    uint16_t rqn; // request number

    uint16_t flg; // flags
    uint16_t arg; // retcode for respond
    uint32_t len; // the length of content 
                  // (max val: 2^32 - sizeof(message_header_t))

    uint8_t data[MESSAGE_RESERVED_LEN]; // Reserved for request/respond
} __attribute__ ((packed));

enum MessageType {
    ADMIN = 0,  // cluster managing
    IO = 1      // input / output
}; // enum ReqType

enum MessageTarget {
    MGR = 1,
    CSD = 2,
    GW = 3
}; // enum ReqTarget

enum MessageFlags {
    // request or respond
    // 0 is req, 1 is res
    REQ_RES = 0x1,
    // used reserved space
    // 1 means that reserved space is been used
    // and len include the size of reserved space.
    // 0 means that len don't include the size of reserved space.
    RESERVED = 0x2,

}; // enum ReqFlags

class Message {
public:
    Message() {}
    ~Message() {}

    // src
    uint16_t src_type() const { return header_.src >> MESSAGE_COMPID_BITLEN; }
    void src_type(uint16_t t) { 
        header_.src = ((uint64_t)t << MESSAGE_COMPID_BITLEN) | (header_.src & MESSAGE_COMPID_MASK);
    }
    uint64_t src_id() const { return header_.src & MESSAGE_COMPID_MASK; }
    void src_id(uint64_t id) {
        header_.src = (header_.src & ~MESSAGE_COMPID_MASK) | (id & MESSAGE_COMPID_MASK);
    }
    uint64_t src() const { return header_.src; }
    void src(uint16_t t, uint64_t id) {
        header_.src = ((uint64_t)t << MESSAGE_COMPID_BITLEN) | (id & MESSAGE_COMPID_MASK);
    }

    // dst
    uint16_t dst_type() const { return header_.dst >> MESSAGE_COMPID_BITLEN; }
    void dst_type(uint16_t t) { 
        header_.dst = ((uint64_t)t << MESSAGE_COMPID_BITLEN) | (header_.dst & MESSAGE_COMPID_MASK);
    }
    uint64_t dst_id() const { return header_.dst & MESSAGE_COMPID_MASK; }
    void dst_id(uint64_t id) {
        header_.dst = (header_.dst & ~MESSAGE_COMPID_MASK) | (id & MESSAGE_COMPID_MASK);
    }
    uint64_t dst() const { return header_.dst; }
    void dst(uint16_t t, uint64_t id) {
        header_.dst = ((uint64_t)t << MESSAGE_COMPID_BITLEN) | (id & MESSAGE_COMPID_MASK);
    }

    // request id
    uint8_t req_typ() const { return header_.typ; }
    void req_typ(uint8_t typ) { header_.typ = typ; }
    uint8_t req_tgt() const { return header_.tgt; }
    void req_tgt(uint8_t tgt) { header_.tgt = tgt; }
    uint8_t req_cls() const { return header_.cls; }
    void req_cls(uint8_t cls) { header_.cls = cls; }
    uint8_t req_num() const { return header_.num; }
    void req_num(uint8_t num) { header_.num = num; }
    void req_id(uint8_t typ, uint8_t tgt, uint8_t cls, uint8_t num) {
        header_.typ = typ;
        header_.tgt = tgt;
        header_.cls = cls;
        header_.num = num;
    }
    void req_id(uint32_t id) { 
        header_.typ = (id & 0xFF000000);
        header_.tgt = (id & 0xFF0000);
        header_.cls = (id & 0xFF00);
        header_.num = (id & 0xFF);
    }
    uint32_t req_id() const {
        return ((uint32_t)header_.typ << 24)
            | ((uint32_t)header_.tgt << 16)
            | ((uint32_t)header_.cls << 8)
            | ((uint32_t)header_.num);
    }
  
    // request group
    uint16_t rqg() const { return header_.rqg; }
    void rqg(uint16_t rqg) { header_.rqg = rqg; }
    // request number
    uint16_t rqn() const { return header_.rqn; }
    void rqn(uint16_t rqn) { header_.rqn = rqn; }

    // flags
    void flags(uint16_t flg, bool v) {
        if (v) header_.flg |= flg;
        else header_.flg &= ~flg;
    }
    bool flags(uint16_t flg) const { return (header_.flg & flg) != 0x0; }
    bool is_req() const { return !flags(MessageFlags::REQ_RES); }
    bool is_res() const { return flags(MessageFlags::REQ_RES); }
    bool reserved_used() const { return flags(MessageFlags::RESERVED); }

    // arg
    uint16_t arg() const { return header_.arg; }
    void arg(uint16_t a) { header_.arg = a; }

    // len
    uint32_t len() const { return header_.len; }
    void len(uint32_t l) { header_.len = l; }

    // Encode the request, from big-end to small-end or from small-end to big-end.
    // Save the result in local.
    // virtual void encode_reverse() {
    //     bytes_reverse_64(header_.src);
    //     bytes_reverse_64(header_.dst);
    //     bytes_reverse_16(header_.rqg);
    //     bytes_reverse_16(header_.rqn);
    //     bytes_reverse_16(header_.flg);
    //     bytes_reverse_16(header_.arg);
    //     bytes_reverse_32(header_.len);
    // }

    //BufferList buff_header() const { return BufferList(BufferPtr(&header_, sizeof(header_))); }
    //virtual BufferList buff_content() { return BufferList(); }

    message_header_t &get_header() { return header_; }

    std::string to_string() const{
        auto s = fmt::format("[Message src {:x} dst{:x} {}[({:p})]",
                    src(), dst(), (req_typ()?"io":"admin"), (void *)this);
        return s;
    }

protected:

    void flag_req() { flags(MessageFlags::REQ_RES, false); }
    void flag_res() { flags(MessageFlags::REQ_RES, true); }

    message_header_t header_;
}; 

typedef std::shared_ptr<Message> MessagePtr;

} //namespace msg
} //namespace flame

#endif // FLAME_MSG_MESSAGE_h