#ifndef FLAME_INCLUDE_CSDS_H
#define FLAME_INCLUDE_CSDS_H

#include "common/context.h"
#include "include/meta.h"
#include "include/callback.h"

#include <memory>
#include <list>

namespace flame {

struct chunk_version_t {
    uint64_t    chk_id;
    uint64_t    epoch;
    uint64_t    version;
    uint64_t    used;  
};

struct chunk_create_attr_t {
    uint64_t    chk_id;
    uint64_t    vol_id;
    uint32_t    index;
    uint32_t    stat;
    uint32_t    spolicy;
    uint64_t    size;
};

struct chunk_move_attr_t {
    uint64_t    chk_id;
    uint64_t    src_id; // src csd id
    uint64_t    dst_id; // dst csd id
    uint32_t    signal; // 1: 迁移; 2: 强制迁移
};

struct chunk_signal_t {
    uint64_t    chk_id;
    uint32_t    signal;
};

class CsdsAsyncChannel;

class CsdsClient {
public:
    virtual ~CsdsClient() {}

    /**
     * 创建一个异步处理Channel
     */
    virtual CsdsAsyncChannel* create_async_channel() const = 0;

    /**
     * Among CSDs
     */
    // 拉取chunk版本信息
    virtual int chunk_fetch(std::list<chunk_version_t>& res, const std::list<uint64_t>& chk_id_list) = 0;

    // 推送Chunk信号：告知Chunk的行为或状态
    virtual int chunk_signal(const std::list<chunk_signal_t>& chk_sgn_list) = 0;

    /**
     * MGR Ctrl
     */
    // 关闭CSD
    virtual int shutdown(uint64_t csd_id) = 0;

    // 清理CSD
    virtual int clean(uint64_t csd_id) = 0;

    // 创建Chunk
    virtual int chunk_create(const chunk_create_attr_t& attr, const std::list<uint64_t>& chk_id_list) = 0;

    // 删除Chunk
    virtual int chunk_remove(uint64_t chk_id) = 0;
    virtual int chunk_remove(const std::list<uint64_t>& chk_id_list) = 0;

    // 选主结果通告
    virtual int chunk_chooss(const std::list<uint64_t>& chk_id_list) = 0;

    // Chunk迁移通告
    virtual int chunk_move(const chunk_move_attr_t& attr) = 0;

protected:
    explicit CsdsClient(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class CsdsClient

typedef void (*csds_async_cb_t)(int res, void* arg);

struct csds_complete_entry_t {
    int res;
    csds_async_cb_t cb;
    void* arg;
};

class CsdsAsyncChannel {
public:
    virtual ~CsdsAsyncChannel() {}

    /**
     * next()
     * 阻塞，只有有一个请求被完成时返回，
     * 返回时不会执行cb，而是将相应结构设置到entry
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int next(csds_complete_entry_t& entry) = 0;
    
    /** 
     * 非阻塞版本的next()
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int try_next(csds_complete_entry_t& entry) = 0;

    /**
     * run_once()
     * 阻塞，直到有一个请求被完成时返回
     * 返回时，相应的cb已经被执行（如果有的话）
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int run_once() = 0;

    /** 
     * 非阻塞版本的run_once()
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int try_once() = 0;

    /**
     * run()
     * 循环等待处理指定数量(limit)完成项，当limit为0时则执行到完成队列为空时返回
     * 相应的cb会被执行
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int run(int limit = 0) = 0;

    /** 
     * 非阻塞版本的run()
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int try_run(int limit = 0) = 0;

    /**
     * Among CSDs
     */
    // 拉取chunk版本信息
    virtual int chunk_fetch(std::list<chunk_version_t>& res, const std::list<uint64_t>& chk_id_list, callback_t cb) = 0;

    // 推送Chunk信号：告知Chunk的行为或状态
    virtual int chunk_signal(const std::list<chunk_signal_t>& chk_sgn_list, callback_t cb) = 0;

    /**
     * MGR Ctrl
     */
    // 关闭CSD
    virtual int shutdown(uint64_t csd_id, callback_t cb) = 0;

    // 清理CSD
    virtual int clean(uint64_t csd_id, callback_t cb) = 0;

    // 创建Chunk
    virtual int chunk_create(const chunk_create_attr_t& attr, const std::list<uint64_t>& chk_id_list, callback_t cb) = 0;

    // 删除Chunk
    virtual int chunk_remove(uint64_t chk_id, callback_t cb) = 0;
    virtual int chunk_remove(const std::list<uint64_t>& chk_id_list, callback_t cb) = 0;

    // 选主结果通告
    virtual int chunk_chooss(const std::list<uint64_t>& chk_id_list, callback_t cb) = 0;

    // Chunk迁移通告
    virtual int chunk_move(const chunk_move_attr_t& attr, callback_t cb) = 0;

protected:
    explicit CsdsAsyncChannel(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class CsdsAsyncChannel

class CsdsClientContext {
public:
    CsdsClientContext(FlameContext* fct, CsdsClient* client)
    : fct_(fct), client_(client) {}

    ~CsdsClientContext() {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }

    CsdsClient* client() const { return client_.get(); }
    void set_client(CsdsClient* c) { client_.reset(c); }


private:
    FlameContext* fct_;
    std::unique_ptr<CsdsClient> client_;
}; // class CsdsClientContext

} // namespace flame

#endif // FLAME_INCLUDE_CSDS_H