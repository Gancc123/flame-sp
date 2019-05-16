/**
 * @file csdc.h
 * @author zhzane (zhzane@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-05-09
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef FLAME_INCLUDE_CSDC_H
#define FLAME_INCLUDE_CSDC_H

#include "include/cmd.h"

#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Chunk IO Commands
 * cls => CMD_CLS_IO_CHK (0xF0U)
 */
#define CMD_CHK_IO_INVALID 0x00U

/**
 * 关于 imm_data_len
 * Chunk Read响应 和 Write请求 可以直接在消息中携带数据（称为立即数据，即imm_data）
 * - imm_data_len 用于指明立即数据的长度
 * - 立即数据的位置必须从 Cache Line 对齐的位置开始
 * - cmd.hdr.len 包含 Command长度 和 立即数据长度
 */

/**
 * chunk io - read & write
 * @res: cmd_res_t
 */
#define CMD_CHK_IO_READ     0x01U
#define CMD_CHK_IO_WRITE    0x02U

struct cmd_chk_io_rd_t {
    chk_id_t    chk_id; // 8 B;
    uint64_t    off;    // 8 B;
    cmd_ma_t    ma;     // 16 B; 访问的数据大小由 dm.len 确定
} __attribute__ ((packed)); // 32 B;

struct res_chk_io_rd_t {
    uint32_t imm_data_len;
} __attribute__ ((packed));

struct cmd_chk_io_wr_t {
    chk_id_t    chk_id; // 8 B;
    uint64_t    off;    // 8 B;
    cmd_ma_t    ma;     // 16 B; 访问的数据大小由 dm.len 确定
    uint32_t    imm_data_len;   // 4 B;
} __attribute__ ((packed)); // 36 B;

/**
 * chunk io - set & reset
 * @res: cmd_res_t
 */
#define CMD_CHK_IO_SET      0x03U   // set every bit to 1
#define CMD_CHK_IO_RESET    0x04U   // set every bit to 0

struct cmd_chk_io_set_t {
    chk_id_t    chk_id; // 8 B;
    uint64_t    off;    // 8 B;
    uint32_t    len;    // 4 B;
} __attribute__((packed));  // 20 B;


#ifdef __cplusplus
}
#endif // __cplusplus

namespace flame {

class ChunkReadCmd : public Command {
public:
    ChunkReadCmd(cmd_t* cmdp) 
    : Command(&rd_cmd_), rd_cmd_(*cmdp), rd_((cmd_chk_io_rd_t*)get_content()) {}

    ChunkReadCmd(uint64_t chk_id, uint64_t off, uint32_t len, const MemoryArea& ma)
    : Command(&rd_cmd_), rd_((cmd_chk_io_rd_t*)get_content()) {
        set_hdr(CMD_CLS_IO_CHK, CMD_CHK_IO_READ, 0, sizeof(cmd_t));
        
        rd_->chk_id = chk_id;
        rd_->off = off;
        rd_->ma.addr = ma.get_addr_uint64();
        rd_->ma.len = len;
        rd_->ma.key = ma.get_key();
    }

    ~ChunkReadCmd() {}

    inline void copy(void* buff) {
        memcpy(buff, (void*)&rd_cmd_, sizeof(cmd_t));
    }

    inline uint64_t get_chk_id() const { return rd_->chk_id; }

    inline uint64_t get_off() const { return rd_->off; }

    inline uint32_t get_ma_len() const { return rd_->ma.len; }

    inline uint64_t get_ma_addr() const { return rd_->ma.addr; }

    inline uint32_t get_ma_key() const { return rd_->ma.key; }

private:
    cmd_t rd_cmd_;
    cmd_chk_io_rd_t* rd_;
}; // class ChunkReadCmd

class ChunkWriteCmd : public Command {
public:
    ChunkWriteCmd(cmd_t* cmdp)
    : Command(&wr_cmd_), wr_cmd_(*cmdp), wr_((cmd_chk_io_wr_t*)get_content()) {
        if (get_imm_data_len() != 0) { // 存在立即数据
            cmd_ = cmdp;
        }
    }

    ChunkWriteCmd(uint64_t chk_id, uint64_t off, uint32_t len, const MemoryArea&  ma, bool forch_imm)
    : Command(&wr_cmd_), wr_((cmd_chk_io_wr_t*)get_content()) {
        set_hdr(CMD_CLS_IO_CHK, CMD_CHK_IO_WRITE, 0, sizeof(cmd_t));
        
        wr_->chk_id = chk_id;
        wr_->off = off;
        wr_->ma.addr = ma.get_addr_uint64();
        wr_->ma.len = len;
        wr_->ma.key = ma.get_key();

        if (!forch_imm || !ma.is_dma()) { // 立即数据传递
            wr_->imm_data_len = ma.get_len();
            set_len(sizeof(cmd_t) + wr_->imm_data_len);
        }
    }

    ~ChunkWriteCmd() {}

    virtual void copy(void* buff) {
        memcpy(buff, &wr_cmd_, sizeof(cmd_t));
        if (get_imm_data_len() != 0) {
            memcpy(buff + sizeof(cmd_t), (void*)wr_->ma.addr, get_imm_data_len());
        }
    }

    inline uint64_t get_chk_id() const { return wr_->chk_id; }

    inline uint64_t get_off() const { return wr_->off; }

    inline uint32_t get_ma_len() const { return wr_->ma.len; }

    inline uint64_t get_ma_addr() const { return wr_->ma.addr; }

    inline uint32_t get_ma_key() const { return wr_->ma.key; }

    inline uint32_t get_imm_data_len() const { return wr_->imm_data_len; }

    inline void* get_imm_data_addr() const { return (void*)cmd_ + sizeof(cmd_t); }

private:
    cmd_t wr_cmd_;
    cmd_chk_io_wr_t* wr_;
}; // class ChunkWriteCmd

class ChunkSetCmd : Command {
public:
    ChunkSetCmd(cmd_t* cmdp) 
    : Command(&set_cmd_), set_cmd_(*cmdp), set_((cmd_chk_io_set_t*)get_content()) {}

    ChunkSetCmd(uint64_t chk_id, uint64_t off, uint32_t len)
    : Command(&set_cmd_), set_((cmd_chk_io_set_t*)get_content()) {
        set_hdr(CMD_CLS_IO_CHK, CMD_CHK_IO_SET, 0, sizeof(cmd_t));
        
        set_->chk_id = chk_id;
        set_->off = off;
        set_->len = len;
    }

    ~ChunkSetCmd() {}

    void copy(void* buff) {
        memcpy(buff, (void*)&set_cmd_, sizeof(cmd_t));
    }

    inline uint64_t get_chk_id() const { return set_->chk_id; }

    inline uint64_t get_off() const { return set_->off; }

    inline uint32_t get_set_len() const { return set_->len; }

private:
    cmd_t set_cmd_;
    cmd_chk_io_set_t* set_;
}; // class ChunkSetCmd

class ChunkResetCmd : Command {
public:
    ChunkResetCmd(cmd_t* cmdp) 
    : Command(&set_cmd_), set_cmd_(*cmdp), set_((cmd_chk_io_set_t*)get_content()) {}

    ChunkResetCmd(uint64_t chk_id, uint64_t off, uint32_t len)
    : Command(&set_cmd_), set_((cmd_chk_io_set_t*)get_content()) {
        set_hdr(CMD_CLS_IO_CHK, CMD_CHK_IO_RESET, 0, sizeof(cmd_t));
        
        set_->chk_id = chk_id;
        set_->off = off;
        set_->len = len;
    }

    ~ChunkResetCmd() {}

    inline void copy(void* buff) {
        memcpy(buff, (void*)&set_cmd_, sizeof(cmd_t));
    }

    inline uint64_t get_chk_id() const { return set_->chk_id; }

    inline uint64_t get_off() const { return set_->off; }

    inline uint32_t get_reset_len() const { return set_->len; }

private:
    cmd_t set_cmd_;
    cmd_chk_io_set_t* set_;
}; // class ChunkSetCmd

class CommonRes : public Response {
public:
    CommonRes(const Command& command, cmd_rc_t rc)
    : Response(&com_res_) {
        cpy_hdr(command);
        set_len(sizeof(cmd_res_t));
        set_rc(rc);
    }

    ~CommonRes() {}

    inline void copy(void* buff) {
        memcpy(buff, &com_res_, sizeof(cmd_res_t));
    }

private:
    cmd_res_t com_res_;
}; // class CommonRes

class ChunkReadRes : public Response {
public:
    ChunkReadRes(const Command& command, cmd_rc_t rc)
    : Response(&rd_res_), imm_data_(nullptr), rd_((res_chk_io_rd_t*)res_->cont) {
        cpy_hdr(command);
        set_len(sizeof(cmd_res_t));
        set_rc(rc);
        rd_->imm_data_len = 0;
    }

    ChunkReadRes(const Command& command, cmd_rc_t rc, void* imm_buff, uint32_t len)
    : Response(&rd_res_), imm_data_(imm_buff), rd_((res_chk_io_rd_t*)res_->cont) {
        cpy_hdr(command);
        set_len(sizeof(cmd_res_t) + len);
        set_rc(rc);
        rd_->imm_data_len = len;
    }

    ~ChunkReadRes() {}

    inline void copy(void* buff) {
        memcpy(buff, &rd_res_, sizeof(cmd_res_t));
        if (imm_data_) {
            memcpy(buff + sizeof(cmd_res_t), imm_data_, rd_->imm_data_len);
        }
    }

private:
    cmd_res_t rd_res_;
    res_chk_io_rd_t* rd_;
    void* imm_data_;
}; // class ChunkReadRes

} // namespace flame


#endif // !FLAME_INCLUDE_CSDC_H