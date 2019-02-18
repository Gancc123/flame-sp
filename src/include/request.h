#ifndef FLAME_INCLUDE_REQUEST_H
#define FLAME_INCLUDE_REQUEST_H

#include <cstdint>

namespace flame {

#define NODE_TYP_SHIFT 56
#define NODE_TYP_MASK (0xFFULL << NODE_TYP_SHIFT)
#define NODE_SUB_ID_MASK (~NODE_TYP_MASK)

struct node_id_t {
    uint64_t val;

    node_id_t(uint64_t node_id) : val(node_id) {}

    node_id_t(uint8_t typ, uint64_t sub_id) 
    : val((uint64_t(typ) << NODE_TYP_SHIFT) | (sub_id & NODE_SUB_ID_MASK)) {}

    void set_type(uint8_t v) { 
        val = ((uint64_t)v << NODE_TYP_SHIFT) | (val & NODE_SUB_ID_MASK); 
    }
    uint8_t get_type() const { return val >> NODE_TYP_SHIFT; }

    void set_sub_id(uint64_t v) { 
        val = (val & NODE_TYP_MASK) | (v & NODE_SUB_ID_MASK);
    }
    uint64_t get_sub_id() const { return val & NODE_SUB_ID_MASK; }

    operator uint64_t () const { return val; }

    node_id_t& operator = (uint64_t v) { val = v; }

    node_id_t& operator = (const node_id_t&) = default;
    node_id_t& operator = (node_id_t&&) = default;
    node_id_t(const node_id_t&) = default;
    node_id_t(node_id_t&&) = default;

} __attribute__ ((packed));

/**
 * @brief 请求号
 * 用于识别确切的请求类型
 * 请求号分为4段，每段一个无符号字节（8 bit)：
 *  - typ   类型
 *  - cls   主分类
 *  - sub   子分类
 *  - idx   序号
 * 注：当前将控制平面与数据平面分离的情况下，请求号只服务于IO请求，
 * 但保留对控制平面的支持
 */
struct req_num_t {
    union {
        uint8_t     part[4];
        uint32_t    number;
    } req;

    req_num_t(uint32_t v) { req.number = v; }
    req_num_t(uint8_t typ, uint8_t cls, uint8_t sub, uint8_t idx) {
        req.part[0] = typ;
        req.part[1] = cls;
        req.part[2] = sub;
        req.part[3] = idx;
    }

    void set_typ(uint8_t v) { req.part[0] = v; }
    uint8_t get_typ() const { return req.part[0]; }

    void set_cls(uint8_t v) { req.part[1] = v; }
    uint8_t get_cls() const { return req.part[1]; }

    void set_sub(uint8_t v) { req.part[2] = v; }
    uint8_t get_sub() const { return req.part[2]; }

    void set_idx(uint8_t v) { req.part[3] = v; }
    uint8_t get_idx() const { return req.part[3]; }

    operator uint32_t () const { return req.number; }

    req_num_t& operator = (uint32_t v) { req.number = v; }

    req_num_t& operator = (const req_num_t&) = default;
    req_num_t& operator = (req_num_t&&) = default;
    req_num_t(const req_num_t&) = default;
    req_num_t(req_num_t&&) = default;

} __attribute__ ((packed));

enum ReqType {
    REQ_TYPE_PUBLIC_ADMIN   = 0x1,
    REQ_TYPE_PUBLIC_IO      = 0x2,
    REQ_TYPE_INTER_ADMIN    = 0x3,
    REQ_TYPE_INTER_IO       = 0x4
};

/**
 * @brief 请求序号
 * 用于唯一标识一个具体的请求
 */
struct req_seq_t {
    uint16_t grp;   // 组号，通常对应队列标识
    uint16_t idx;   // 序号，通常对应队列中的偏移
} __attribute__ ((packed));

struct request_header_t {
    req_num_t   num;    // 32 bit
    req_seq_t   seq;    // 32 bit
    node_id_t   dst;    // 64 bit
} __attribute__ ((packed));


/**
 * Chunk Request
 */
struct chunk_request_t {
    uint8_t     stat;
    uint8_t     grp;    // 类似于req_seq_t.grp
    uint16_t    seq;    // 类似于req_seq_t.seq，用于标识所属上层请求
    req_num_t   num;    // 32 bit
    uint64_t    chk_id; 
    uint32_t    lba;    // 以 4KB Page为单位
    uint32_t    lba_cnt;    
} __attribute__ ((packed));

class ChunkRequest {
public:
    virtual ~ChunkRequest() {}

protected:
    ChunkRequest() {}
};

} // namespace flame

#endif // FLAME_INCLUDE_REQUEST_H