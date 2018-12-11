#ifndef FLAME_SERVICE_CLIENT_H
#define FLAME_SERVICE_CLIENT_H

#include "include/csds.h"
#include "common/context.h"

#include <grpcpp/grpcpp.h>
#include "proto/csds.grpc.pb.h"

#include <memory>
#include <string>

namespace flame {

class CsdsClientImpl final : public CsdsClient {
public:
    CsdsClientImpl(FlameContext* fct, std::shared_ptr<grpc::Channel> channel)
    : CsdsClient(fct), stub_(CsdsService::NewStub(channel)) {}

    /**
     * 创建一个异步处理Channel
     */
    virtual CsdsAsyncChannel* create_async_channel() const override;

    /**
     * Among CSDs
     */
    // 拉取chunk版本信息
    virtual int chunk_fetch(std::list<chunk_version_t>& res, const std::list<uint64_t>& chk_id_list) override;

    // 推送Chunk信号：告知Chunk的行为或状态
    virtual int chunk_signal(const std::list<chunk_signal_t>& chk_sgn_list) override;

    /**
     * MGR Ctrl
     */
    // 关闭CSD
    virtual int shutdown(uint64_t csd_id) override;

    // 清理CSD
    virtual int clean(uint64_t csd_id) override;

    // 创建Chunk
    virtual int chunk_create(const chunk_create_attr_t& attr) override;

    // 删除Chunk
    virtual int chunk_remove(uint64_t chk_id) override;
    virtual int chunk_remove(const std::list<uint64_t>& chk_id_list) override;

    // 选主结果通告
    virtual int chunk_chooss(const std::list<uint64_t>& chk_id_list) override;

    // Chunk迁移通告
    virtual int chunk_move(const chunk_move_attr_t& attr) override;

private:
    std::unique_ptr<CsdsService::Stub> stub_;
}; // class CsdsClient

enum CsdsRqn {
    RQN_CHUNK_FETCH = 1,
    RQN_CHUNK_SIGNAL = 2,
    RQN_SHUTDOWN = 3,
    RQN_CLEAN = 4,
    RQN_CHUNK_CREATE = 5,
    RQN_CHUNK_REMOVE = 6,
    RQN_CHUNK_REMOVE_M = 7,
    RQN_CHUNK_MOVE = 8
};

enum CsdsRpt {
    RPT_REPLY = 1,
    RPT_CHUNK_FETCH = 2
};

struct csds_req_entry_t {
    int rqn         {0};    // request num
    void* res       {nullptr};
    int rpt         {0};    // reply type
    void* reply     {nullptr};
    csds_complete_entry_t ce;

    ~csds_req_entry_t() { delete reply; }
};

class CsdsAsyncChannelImpl final : public CsdsAsyncChannel {
public:
    /**
     * next()
     * 阻塞，只有有一个请求被完成时返回，
     * 返回时不会执行cb，而是将相应结构设置到entry
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int next(csds_complete_entry_t& entry) override;
    
    /** 
     * 非阻塞版本的next()
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int try_next(csds_complete_entry_t& entry) override;

    /**
     * run_once()
     * 阻塞，直到有一个请求被完成时返回
     * 返回时，相应的cb已经被执行（如果有的话）
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int run_once() override;

    /** 
     * 非阻塞版本的run_once()
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int try_once() override;

    /**
     * run()
     * 循环等待处理指定数量(limit)完成项，当limit为0时则执行到完成队列为空时返回
     * 相应的cb会被执行
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int run(int limit = 0) override;

    /** 
     * 非阻塞版本的run()
     * @return: >= 0 表示获取的完成项的数量，< 0 表示错误信息
     */
    virtual int try_run(int limit = 0) override;

    /**
     * Among CSDs
     */
    // 拉取chunk版本信息
    virtual int chunk_fetch(std::list<chunk_version_t>& res, const std::list<uint64_t>& chk_id_list, csds_async_cb_t cb, void* arg) override;

    // 推送Chunk信号：告知Chunk的行为或状态
    virtual int chunk_signal(const std::list<chunk_signal_t>& chk_sgn_list, csds_async_cb_t cb, void* arg) override;

    /**
     * MGR Ctrl
     */
    // 关闭CSD
    virtual int shutdown(uint64_t csd_id, csds_async_cb_t cb, void* arg) override;

    // 清理CSD
    virtual int clean(uint64_t csd_id, csds_async_cb_t cb, void* arg) override;

    // 创建Chunk
    virtual int chunk_create(const chunk_create_attr_t& attr, csds_async_cb_t cb, void* arg) override;

    // 删除Chunk
    virtual int chunk_remove(uint64_t chk_id, csds_async_cb_t cb, void* arg) override;
    virtual int chunk_remove(const std::list<uint64_t>& chk_id_list, csds_async_cb_t cb, void* arg) override;

    // 选主结果通告
    virtual int chunk_chooss(const std::list<uint64_t>& chk_id_list, csds_async_cb_t cb, void* arg) override;

    // Chunk迁移通告
    virtual int chunk_move(const chunk_move_attr_t& attr, csds_async_cb_t cb, void* arg) override;

private:
    CsdsAsyncChannelImpl(FlameContext* fct, std::shared_ptr<Channel> channel)
    : CsdsAsyncChannel(fct), stub_(CsdsService::NewStub(channel)) {}

    std::unique_ptr<CsdsService::Stub> stub_;
}; // class CsdsAsyncChannelImpl

} // namespace flame

#endif // FLAME_SERVICE_CLIENT_H