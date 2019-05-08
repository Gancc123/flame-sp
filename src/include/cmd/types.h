/**
 * @file types.h
 * @author your name (you@domain.com)
 * @brief 未完成
 * @version 0.1
 * @date 2019-05-06
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef FLAME_INCLUDE_CMD_TYPES_H
#define FLAME_INCLUDE_CMD_TYPES_H

#define CACHE_LINE_SIZE 64

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Command Base 
 */

/**
 * Command Number
 * @length: 16 bit (2 Bytes)
 */
union cmd_num_t {
    uint8_t cls;    // command class
    uint8_t seq;    // command sequence
} __attribute__((packed));

// 控制平面（预留类型）
#define CMD_CLS_MGR 0x00U   // 0x00 ~ 0x0F for MGR
#define CMD_CLS_CSD 0x10U   // 0x10 ~ 0x1F for CSD
#define CMD_CLS_TGT 0x20U   // 0x20 ~ 0x2F for TGT / GW

// 数据平面
#define CMD_CLS_IO_CHK 0xF0U    // Chunk

/**
 * Command Header
 * @length: 64 bit (8 Bytes)
 */
struct cmd_hdr_t {
    cmd_num_t   cn;     // 16 bit;
    uint8_t     flg;    // 8 bit; Flags. 
    uint8_t     cqg;    // 8 bit; Command Queue Group. 最多支持 256 个队列
    uint16_t    cqn;    // 16 bit; Command Queue Number. 队列深度最大为 64K
    uint16_t    len;    // 16 bit; 以 CACHE_LINE_SIZE 为单位
} __attribute__((packed));

/**
 * Chunk ID
 * @length: 64 bit (8 Bytes)
 */
typedef uint64_t chk_id_t;

/**
 * Chunk Offset
 * @length: 32 bit (4 Bytes)
 * 以 CACHE_LINE_SIZE 为单位，因此支持最大的 Chunk 尺寸为 256 GB
 */
typedef uint32_t chk_off_t;

#define chk_off_to_real(off) ((uint64_t)off << 6)

/**
 * Command Return Code
 * @length: 
 */
typedef uint16_t cmd_rc_t;

/**
 * Command Data Memory
 * @length: 16 Bytes
 * 指定所操作的内存地址空间
 */
struct cmd_dm_t {
    uint64_t addr;
    uint32_t len;
    uint32_t key;   // 秘钥，对应 RDMA 的 lkey/rkey
} __attribute__((packed));

/**
 * Empty Command
 * @length: 64 Bytes
 */
#define CMD_RESERVED_SIZE (CACHE_LINE_SIZE - sizeof(struct cmd_hdr_t))

struct cmd_t {
    cmd_hdr_t   hdr;    // 8 Bytes
    uint8_t     cont[CMD_RESERVED_SIZE];
} __attribute__((packed));

/**
 * Emtpy Command Response
 * 
 */
#define CMD_RES_RESERVED_SIZE (CACHE_LINE_SIZE - sizeof(struct cmd_hdr_t) - sizeof(cmd_rc_t))

struct cmd_res_t {
    cmd_hdr_t   hdr;    // 8 Bytes
    cmd_rc_t    rc;     // 2 Bytes
    uint8_t     cont[CMD_RES_RESERVED_SIZE];
} __attribute__((packed));

/**
 * Command Callback
 */
typedef void (*cmd_fn_t)(void* arg, struct cmd_t* cmd, void* res, cmd_rc_t rc);

/**
 * Chunk IO Commands
 * @cls: CMD_CLS_IO_CHK (0xF0U)
 */
#define CMD_CHK_IO_INVALID 0x00U

/**
 * chunk io - read & write
 * @res: cmd_res_t
 */
#define CMD_CHK_IO_READ     0x01U
#define CMD_CHK_IO_WRITE    0x02U

struct req_chk_io_rw_t {
    chk_id_t    chk_id; // 8 B;
    chk_off_t   off;    // 4 B;
    cmd_dm_t    dm;     // 16 B; 访问的数据大小由 dm.len 确定
    uint32_t    imm_data_len;   // 4 B; 
} __attribute__((packed)); // 32 B;

/**
 * chunk io - set & reset
 * @res: cmd_res_t
 */
#define CMD_CHK_IO_SET      0x03U   // set every bit to 1
#define CMD_CHK_IO_RESET    0x04U   // set every bit to 0

struct req_chk_io_set_t {
    chk_id_t    chk_id; // 8 B;
    chk_off_t   off;    // 4 B;
    uint32_t    len;    // 4 B;
} __attribute__((packed));  // 16 B;

#ifdef __cplusplus
}
#endif

#endif // FLAME_INCLUDE_CMD_TYPES_H
