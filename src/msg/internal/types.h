#ifndef FLAME_MSG_INTERNAL_TYPES_H
#define FLAME_MSG_INTERNAL_TYPES_H

#include "int_types.h"

namespace flame{
namespace msg{

/**
 * Node Addr
 */
struct msg_node_addr_t {
    uint8_t     ttype;      // transport type
    uint8_t     addr_type;
    uint16_t    port;
    union {
        uint8_t     ipv6[16];
        uint32_t    ipv4;
    } addr;
} __attribute__((packed));

#define NODE_ADDR_TTYPE_UNKNOWN 0x00
#define NODE_ADDR_TTYPE_TCP     0x01
#define NODE_ADDR_TTYPE_RDMA    0x02

#define NODE_ADDR_ADDRTYPE_UNKNOWN 0x00
#define NODE_ADDR_ADDRTYPE_IPV4    0x01
#define NODE_ADDR_ADDRTYPE_IPV6    0x02

struct flame_msg_header_t{
    __u8    type;
    __u8    flag;
    __u8    priority;
    __u8    reserved; 
    uint32_t len;
} __attribute__ ((packed));

#define FLAME_MSG_HEADER_LEN (sizeof(struct flame_msg_header_t))

#define FLAME_MSG_PRIO_LOW     64
#define FLAME_MSG_PRIO_DEFAULT 127
#define FLAME_MSG_PRIO_HIGH    196
#define FLAME_MSG_PRIO_HIGHEST 255

#define FLAME_MSG_TYPE_NONE               0
#define FLAME_MSG_TYPE_CTL                1
#define FLAME_MSG_TYPE_IO                 2
#define FLAME_MSG_TYPE_DECLARE_ID         3
#define FLAME_MSG_TYPE_IMM_DATA           4

#define FLAME_MSG_FLAG_RESP               1
#define FLAME_MSG_FLAG_RDMA               (1 << 1)//has flame_msg_rdma_header_t.
#define FLAME_MSG_FLAG_MEM_FETCH          (1 << 2)//when unset, means mem push.
#define FLAME_MSG_FLAG_WITH_IMM           (1 << 3)//with 4 Bytes imm data.

#define FLAME_MSG_RDMA_MEM_PUSH      0
#define FLAME_MSG_RDMA_MEM_FETCH     1

#define FLAME_MSG_RDMA_TYPE(flag)     (((flag) >> 2) & 1)
#define FLAME_MSG_RDMA_CNT(reserved)  (reserved) 

struct flame_msg_imm_data_t{
    __le32 imm_data;
    char __pad[4];
} __attribute__ ((packed));

struct flame_msg_rdma_mr_t{
    __le64 addr;
    __le32 rkey;
    __le32 len;
} __attribute__ ((packed));


struct msger_id_t {
    uint32_t ip     {0};    // just for ipv4
    uint16_t port   {0};  // port
} __attribute__((packed));

struct msg_hdr_t {
    uint16_t len; //以64为单位
    uint8_t  cls; //0x30~0x3F
    uint8_t  opcode;
} __attribute__((packed));

#define FLAME_MSG_CMD_CLS_MSG 0x30U

#define FLAME_MSG_HDR_OPCODE_DECLARE_ID 0x1

#define FLAME_MSG_CMD_RESERVED_LEN (64 - sizeof(struct msg_hdr_t))

struct msg_cmd_t {
    struct msg_hdr_t mh;
    char content[FLAME_MSG_CMD_RESERVED_LEN];
} __attribute__((packed));

struct msg_cmd_delcared_id_t{
    struct msger_id_t msger_id;
    //flags = (has_tcp_lp | has_rdma_lp) : has_tcp_lp => 1, has_rdma_lp => 2
    char flags; 
    msg_node_addr_t addr1;
    msg_node_addr_t addr2;
} __attribute__((packed));


} //namespace msg
} //namespace flame

#endif //FLAME_MSG_INTERNAL_TYPES_H