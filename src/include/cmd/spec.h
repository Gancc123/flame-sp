/**
 * @file spec.h
 * @author your name (you@domain.com)
 * @brief 未完成
 * @version 0.1
 * @date 2019-05-06
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef FLAME_INCLUDE_LIBCHUNK_H
#define FLAME_INCLUDE_LIBCHUNK_H

#include "include/cmd/types.h"

#include <cstdint>
#include <string>

namespace flame {

class Command {
public:
    inline uint16_t get_num() const { return *(uint16_t*)&cmd_->hdr.cn; }
    inline uint8_t get_cqg() const { return cmd_->hdr.cqg; }
    inline uint16_t get_cqn() const { return cmd_->hdr.cqn; }
    inline uint64_t get_len() const { return chk_off_to_real(cmd_->hdr.len); }

    inline void set_num(uint8_t cls, uint8_t seq) {
        cmd_->hdr.cn.cls = cls;
        cmd_->hdr.cn.seq = seq;
    }

    inline void set_cqg(uint8_t cqg) { cmd_->hdr.cqg = cqg; }
    inline void set_cqn(uint16_t cqn) { cmd_->hdr.cqn = cqn; }

protected:
    Command(cmd_t* cmdp) : cmd_(cmdp) {}
    virtual ~Command() {}

    inline void set_len(uint16_t len) { cmd_->hdr.len = len; }

    /**
     * @brief Set the len real object
     * 计算成 Cache Line对齐的方式
     * @param len 
     */
    inline void set_len_real(uint32_t len) {
        set_len(len % CACHE_LINE_SIZE ? len / CACHE_LINE_SIZE + 1 : len / CACHE_LINE_SIZE);
    }

    cmd_t* cmd_;
}; // class Command

/**
 * @brief 这是一个对连接的抽象，这里的“连接”用于标识GW与CSD之间的通路
 * 
 */
class Connection {
public:
    virtual uint64_t get_node_id() const = 0;
    
    virtual std::string get_node_name() const = 0;

    /**
     * submit_cmd()
     * Submit Command
     * @param cmd: Command
     * @param cb_fn: 
     * @param cb_arg:
     * @param timeout: (us), 0 => waitting forever
     */
    virtual int submit_cmd(const Command& cmd, cmd_fn_t cb_fn, void* cb_arg, uint64_t timeout) = 0;

protected:
    Connection() {}
    virtual ~Connection() {}
}; // class Connection

class DataArea {
public:
    /**
     * is_inline()
     * 表示该数据块内嵌在命令尾部传输，而不是单独传输
     * 通常针对RDMA，但也可以一般性地用于实现命令头部和数据的分离传输
     * （对于非RDMA内存区域，无论大小，只能是内联的方式）
     */
    virtual bool is_inline() const = 0;

    virtual uint64_t get_addr() const = 0;
    virtual uint32_t get_len() const = 0;
    virtual uint32_t get_key() const = 0;

    virtual cmd_dm_t get_dm() const = 0;

protected:
    DataArea() {}
    virtual ~DataArea() {}
};

/**
 * Commands
 */
class CmdChunkRead : public Command {
public:
    CmdChunkRead(uint64_t chk_id, uint64_t off, uint64_t len, const DataArea& da)
    : Command(&rd_cmd_), rw_(&rd_cmd_.cont) {
        rd_cmd_.hdr.cn.cls = CMD_CLS_IO_CHK;
        rd_cmd_.hdr.cn.seq = CMD_CHK_IO_READ;
        rd_cmd_.hdr.flg = 0;
        rd_cmd_.hdr.len = sizeof(cmd_t);    // ChunkRead命令的大小是固定的

        rw_->chk_id = chk_id;
        rw_->off = off;
        rw_->len = len;
        rw_->dm = da.get_dm();
        rw_->imm_data_len = da.is_inline() ? da.get_len() : 0;
    }
    virtual ~Command() {}

protected:
    cmd_t rd_cmd_;
    const req_chk_io_rw_t* rw_;
}; // class CmdChunkRead

} // namespace flame


#endif // FLAME_INCLUDE_LIBCHUNK_H