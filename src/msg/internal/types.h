#ifndef FLAME_MSG_INTERNAL_TYPES_H
#define FLAME_MSG_INTERNAL_TYPES_H

#include "int_types.h"

/**
 * Node Addr
 */
struct node_addr_t {
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
    __le32  len;
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

#define FLAME_MSG_FLAG_RESP               1
#define FLAME_MSG_FLAG_RDMA               (1 << 1)//has flame_msg_rdma_header_t.
#define FLAME_MSG_FLAG_MEM_FETCH          (1 << 2)//when unset, means mem push.

#define FLAME_MSG_RDMA_MEM_PUSH      0
#define FLAME_MSG_RDMA_MEM_FETCH     1

#define FLAME_MSG_RDMA_TYPE(flag)     (((flag) >> 2) & 1)
#define FLAME_MSG_RDMA_CNT(reserved)  (reserved) 

struct flame_msg_rdma_mr_t{
    __le64 addr;
    __le32 rkey;
    __le32 len;
} __attribute__ ((packed));

struct msger_id_t {
    unsigned long ip; // just for ipv4
    unsigned short port; // port
} __attribute__((packed));

#endif //FLAME_MSG_INTERNAL_TYPES_H