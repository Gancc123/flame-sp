#ifndef FLAME_CHUNKSTORE_H
#define FLAME_CHUNKSTORE_H

#include "common/context.h"
#include "include/meta.h"

#include <cstdint>
#include <string>
#include <memory>

namespace flame {

/**
 * ChunkStore Infomation
 */
struct cs_info_t {
    uint64_t    id;
    std::string cluster_name;
    std::string name;
    uint64_t    size;
    uint64_t    used;
    uint64_t    ftime;    // format time (us)
    uint32_t    chk_num;
}; // cs_info_t

/**
 * Chunk Information
 */
struct chunk_info_t {
    uint64_t    chk_id;
    uint64_t    vol_id;
    uint32_t    index;
    uint32_t    stat;
    uint32_t    spolicy;
    uint32_t    flags;
    uint64_t    size;
    uint64_t    used;
    uint64_t    ctime;
    uint64_t    dst_id;    // 只在状态为迁移时有效
    uint64_t    dst_ctime;
}; // struct chunk_info_t

struct chunk_create_opts_t {
    uint64_t    vol_id      {0};
    uint32_t    index       {0};
    uint32_t    spolicy     {0};
    uint32_t    flags       {0};
    uint64_t    size        {1ULL << 32}; // 默认4GB

    bool is_prealloc() const {
        return flags & CHK_FLG_PREALLOC;
    }

    void set_prealloc(bool v) {
        if (v) flags |= CHK_FLG_PREALLOC;
        else flags &= ~CHK_FLG_PREALLOC;
    }
}; // chunk_create_opts_t

/**
 * chunk_opt_cb_t
 * 用于Chunk异步读写的回调
 */
typedef void (*chunk_opt_cb_t) (void* arg);

struct chunk_async_opt_entry_t {
    void* buff;
    uint64_t off;
    uint64_t len;
    chunk_opt_cb_t cb;
    void* cb_arg;
};

class Chunk {
public:
    virtual ~Chunk() {}

    /**
     * 基本信息部分
     */

    /**
     * get_info()
     * 获取Chunk元数据信息
     * @return: 获取成功返回0
     */
    virtual int get_info(chunk_info_t& info) const = 0;

    /**
     * size()
     * @return: the size of chunk
     */
    virtual uint64_t size() const = 0;

    /**
     * used()
     * @return: used space of chunk
     */
    virtual uint64_t used() const = 0;

    /**
     * stat()
     * @return: the status of chunk
     */
    virtual uint32_t stat() const = 0;

    /**
     * vol_id()
     * @return: 
     */
    virtual uint64_t vol_id() const = 0;

    /**
     * vol_index()
     * @return
     */
    virtual uint32_t vol_index() const = 0;

    /**
     * splociy()
     * @return: store policy
     */
    virtual uint32_t spolicy() const = 0;

    /**
     * is_preallocated()
     * @return: True iff this chunk is setted with pre allocated
     */
    virtual bool is_preallocated() const = 0;

    /**
     * 操作部分
     */

    /**
     * read_sync()
     * 同步读
     */
    virtual int read_sync(void* buff, uint64_t off, uint64_t len) = 0;

    /**
     * write_sync()
     * 同步写
     */
    virtual int write_sync(void* buff, uint64_t off, uint64_t len) = 0;

    /**
     * get_xattr()
     * 同步获取扩展属性
     */
    virtual int get_xattr(const std::string& name, std::string& value) = 0;

    /**
     * set_xattr()
     * 同步设置扩展属性
     */
    virtual int set_xattr(const std::string& name, const std::string& value) = 0;

    /**
     * read_async()
     * 异步读
     */
    virtual int read_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) = 0;

    /**
     * write_async()
     * 异步写
     */
    virtual int write_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) = 0;

    Chunk(const Chunk&) = delete;
    Chunk(Chunk&&) = delete;
    Chunk& operator = (const Chunk&) = delete;
    Chunk& operator = (Chunk&&) = delete;

protected:
    Chunk(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class Chunk

/**
 * Interface for ChunkStore
 */
class ChunkStore {
public:
    virtual ~ChunkStore() {}

    /**
     * 基础信息部分
     */

    /**
     * @brief 设置ChunkStore基础信息
     * 新CSD注册时，需要更新并持久化基础信息，包括ID，即CSD_ID
     * 注：
     *  - 通过获取info，判断ID是否为0，来验证是否已经注册
     *  - 此外还会验证集群信息
     *  - 部分信息可以被忽略，如size, used等，主要设置cluster_name, id, name
     * @param info 
     * @return int 
     */
    virtual int set_info(const cs_info_t& info) = 0;

    /**
     * get_info()
     * 获取ChunkStore的元数据信息
     * @return: 获取成功返回0
     */
    virtual int get_info(cs_info_t& info) const = 0;

    /**
     * get_driver_name()
     */
    virtual std::string get_driver_name() const = 0;

    /**
     * get_config_info()
     * @return: 返回ChunkStore的配置信息
     */
    virtual std::string get_config_info() const = 0;

    /**
     * get_rumtime_info()
     * @return: 返回MetaStore的运行时信息，指明内部状态
     */
    virtual std::string get_runtime_info() const = 0;
    
    /**
     * get_io_mode()
     * @return: the io mode of chunkstore, IOMode
     */
    virtual int get_io_mode() const = 0;
    enum IOMode {
        SYNC = 0,
        ASYNC = 1
    };

    /**
     * is_support_mem_persist()
     * 为MemStore和支持局部内存持久的ChunkStore预留接口
     * 默认不支持，故而返回false
     * @return: 如果支持内存持久化则返回true
     */
    virtual bool is_support_mem_persist() const { return false; }

    /**
     * 设备挂载部分
     */

    /**
     * dev_check()
     * 检查设备状态（对于FileStore则是目录）：
     *      1. 是否已经有有效数据；
     *      2. 是否属于所配置的集群；
     * @return: DevStatus
     */
    virtual int dev_check() = 0;
    enum DevStatus {
        NONE = 0,       // 空设备/目录 （可能很难检测设备为空）
        UNKNOWN = 1,    // 未知结构数据
        CLT_OUT = 2,    // 有效的Flame CSD数据，但不属于所配置集群
        CLT_IN = 3      // 有效的Flame CSD数据并且属于所配置集群
    };

    enum RetCode {
        SUCCESS = 0,
        FAILD = 1,
        OBJ_NOTFOUND = 2,
        OBJ_EXISTED = 3
    };

    /**
     * dev_format()
     * 格式化设备：无论是否有数据，都将其清空并重新格式化
     * 如果不需要强制格式化，应该先调研dev_check()检查设备状态
     * @return: 0 is success
     */
    virtual int dev_format() = 0;

    /**
     * dev_mount()
     * 挂载设备，也是初始化ChunkStore的一个主要步骤，在此阶段加载相应数据（如超级块）
     * 可以不执行dev_check和dev_format直接执行dev_mount，但可能返回错误
     * @return: 0 is success
     */
    virtual int dev_mount() = 0;

    /**
     * dev_umount()
     * 卸载设备，关闭ChunkStore
     * 将主元数据信息写回
     * @return: 0 is success
     */
    virtual int dev_unmount() = 0;

    /**
     * is_mounted()
     * 返回是否已经挂载成功
     */
    virtual bool is_mounted() = 0;

    /**
     * Chunk操作部分
     */

    // 创建Chunk
    virtual int chunk_create(uint64_t chk_id, const chunk_create_opts_t& opts) = 0;

    // 删除Chunk
    virtual int chunk_remove(uint64_t chk_id) = 0;

    // 查看Chunk是否已经存在
    virtual bool chunk_exist(uint64_t chk_id) = 0;

    // 打开一个Chunk，不存在时返回空指针
    virtual std::shared_ptr<Chunk> chunk_open(uint64_t chk_id) = 0;

    ChunkStore(const ChunkStore&) = delete;
    ChunkStore(ChunkStore&&) = delete;
    ChunkStore& operator = (const ChunkStore&) = delete;
    ChunkStore& operator = (ChunkStore&&) = delete;

protected:
    ChunkStore(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class ChunkHandle

} // namespace flame

#endif // FLAME_CHUNKSTORE_H