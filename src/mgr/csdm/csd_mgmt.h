#ifndef FLAME_MGR_CSDM_H
#define FLAME_MGR_CSDM_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "include/objects.h"
#include "common/thread/rw_lock.h"
#include "include/internal.h"
#include "include/csds.h"
#include "layout/calculator.h"

#include <cstdint>
#include <atomic>
#include <memory>
#include <map>

namespace flame {

class CsdHandle;

class CsdManager final {
public:
    CsdManager(FlameContext* fct, 
        const std::shared_ptr<MetaStore>& ms, 
        const std::shared_ptr<CsdsClientFoctory>& csd_client_foctory,
        const std::shared_ptr<layout::CsdHealthCaculator>& csd_hlt_calor)
    : fct_(fct), ms_(ms), csd_client_foctory_(csd_client_foctory), csd_hlt_calor_(csd_hlt_calor) {}
    
    ~CsdManager() {
        destroy__();
    }

    /**
     * @brief 初始化CsdManager
     * 从MetaSotre加载所有CSD信息
     * 只在初始化时被使用
     * @return int 
     */
    int init();
    
    /**
     * @brief CSD注册
     * 
     * @param attr 
     * @param hp 
     * @return int 
     */
    int csd_register(const csd_reg_attr_t& attr, CsdHandle** hp);

    /**
     * @brief CSD注销
     * 
     * @param csd_id 
     * @return int 
     */
    int csd_unregister(uint64_t csd_id);

    /**
     * @brief CSD上线
     * 
     * @param attr 
     * @return int 
     */
    int csd_sign_up(const csd_sgup_attr_t& attr);

    /**
     * @brief CSD下线
     * 
     * @param csd_id 
     * @return int 
     */
    int csd_sign_out(uint64_t csd_id);

    // int csd_heart_beat(uint64_t csd_id);

    /**
     * @brief 更新CSD状态
     * 
     * @param csd_id 
     * @param stat 
     * @return int 
     */
    int csd_stat_update(uint64_t csd_id, uint32_t stat);

    /**
     * @brief 更新CSD健康信息
     * 
     * @param csd_id 
     * @param hlt 
     * @return int 
     */
    int csd_health_update(uint64_t csd_id, const csd_hlt_sub_t& hlt);

    /**
     * @brief 拉取CSD地址信息
     * 
     * @param addrs 
     * @param csd_ids 为空时拉取全部CSD地址信息
     * @return int 
     */
    int csd_pull_addr(std::list<csd_addr_t>& addrs, const std::list<uint64_t>& csd_ids);

    /**
     * @brief 查询CSD
     * 
     * @param csd_id 
     * @return CsdHandle* 
     */
    CsdHandle* find(uint64_t csd_id);

    /**
     * @brief Get the lock object
     * 
     * @return RWLock& 
     */
    RWLock& get_lock() { return csd_map_lock_; }
    void read_lock() { csd_map_lock_.rdlock(); }
    void unlock() { csd_map_lock_.unlock(); }

    /**
     * @brief 
     * 
     * @return std::map<uint64_t, CsdHandle*>::const_iterator 
     */
    std::map<uint64_t, CsdHandle*>::const_iterator csd_hdl_begin() const { return csd_map_.cbegin(); }
    std::map<uint64_t, CsdHandle*>::const_iterator csd_hdl_end() const { return csd_map_.cend(); }

    friend class CsdHandle;
private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    std::shared_ptr<CsdsClientFoctory> csd_client_foctory_;
    std::shared_ptr<layout::CsdHealthCaculator> csd_hlt_calor_;

    /**
     * CsdHandle 存储管理
     */
    std::map<uint64_t, CsdHandle*> csd_map_;
    RWLock csd_map_lock_;
    std::atomic<uint64_t> next_csd_id_;

    /**
     * @brief Create a csd handle object
     * 不会插入csd_map_
     * @param csd_id 
     * @return CsdHandle* 
     */
    CsdHandle* create_csd_handle__(uint64_t csd_id);

    /**
     * @brief 将CsdHandle插入csd_map_
     * 
     * @param csd_id 
     * @param hdl 
     * @return true 
     * @return false 
     */
    bool insert_csd_handle__(uint64_t csd_id, CsdHandle* hdl);

    /**
     * @brief 销毁csd_map_
     * 
     */
    void destroy__();
}; // class CsdManager

class CsdHandle final {
public:
    CsdHandle(CsdManager* csdm, const std::shared_ptr<MetaStore>& ms, uint64_t csd_id) 
    : csdm_(csdm), csd_id_(csd_id), stat_(CSD_OBJ_STAT_LOAD), ms_(ms) {
        obj_ = new CsdObject();
    }

    ~CsdHandle() { delete obj_; }

    /**
     * @brief CSD状态
     * 
     * @return int 
     */
    int stat() const { return stat_.load(); }

    bool is_down() const { return stat_.load() == CSD_STAT_DOWN; }
    bool is_pause() const { return stat_.load() == CSD_STAT_PAUSE; }
    bool is_active() const { return stat_.load() == CSD_STAT_ACTIVE; }

    /**
     * @brief 更新CSD状态
     * 
     * @param st 
     */
    void update_stat(int st);

    /**
     * @brief CSD最后活跃时间
     * 
     * @return utime_t 
     */
    utime_t latime() { return latime_; }
    
    /**
     * @brief 更新健康信息
     * 
     * @param hlt 
     */
    void update_health(const csd_hlt_sub_t& hlt);

    /**
     * @brief 判断是否已经建立连接
     * 
     * @return true 
     * @return false 
     */
    bool is_connected() { return client_.get() != nullptr; }

    /**
     * @brief 获取客户端
     * 
     * @return std::shared_ptr<CsdsClient> 
     */
    std::shared_ptr<CsdsClient> get_client();

    /**
     * @brief 获取CsdObject，未加锁
     * 
     * @return CsdObject* 
     */
    CsdObject* get();

    /**
     * @brief 获取Handle的读写锁
     * 
     * @return RWLock& 
     */
    RWLock& get_lock() { return lock_; }

    /**
     * @brief 获取对象并加锁
     * 需要通过unlock释放锁
     * 
     * @return CsdObject* 
     */
    CsdObject* read_and_lock();
    CsdObject* read_try_lock();
    CsdObject* write_and_lock();
    CsdObject* write_try_lock();
    void unlock() { lock_.unlock(); }

    /**
     * @brief 持久化 CsdObject 并释放锁
     * 需要在 read_and_lock()或write_and_lock()后使用，
     * 或者在 read_try_lock()或write_try_lock()成功后使用
     */
    int save_and_unlock();

    /**
     * @brief 持久化 CsdObject
     * 需要持有锁，读写锁皆可
     * @return int 0 => success, 1 => faild
     */
    int save();

    /**
     * @brief 从MetaStore加载CsdObject
     * 需要持有写锁
     * @return int 
     */
    int load();
    
    /**
     * @brief 连接CSD并创建客户端句柄
     * 需要持有锁
     * @return int 
     */
    int connect();

    /**
     * @brief 断开与CSD的连接
     * 需要持有锁
     * @return int 
     */
    int disconnect();

    friend class CsdManager;
private:
    CsdManager* csdm_;

    uint64_t csd_id_;
    CsdObject* obj_ {nullptr};

    std::atomic<int> stat_ {CSD_STAT_DOWN};
    utime_t latime_;    // last active time
    void set_as_none__() { stat_ = CSD_STAT_NONE; }
    void set_as_down__() { stat_ = CSD_STAT_DOWN; }
    void set_as_pause__() { stat_ = CSD_STAT_PAUSE; }
    void set_as_active__() { stat_ = CSD_STAT_ACTIVE; }

    void set_latime__(utime_t ut) { latime_ = ut; }
    
    enum CsdObjStat {
        CSD_OBJ_STAT_NEW  = 0,  // 新创建的状态，尚未保存
        CSD_OBJ_STAT_LOAD = 1,  // 处于加载状态
        CSD_OBJ_STAT_SVAE = 2,  // 已保存状态
        CSD_OBJ_STAT_DIRT = 3,  // 数据脏，等待保存
        CSD_OBJ_STAT_TRIM = 4   // 等待删除
    };
    std::atomic<int> obj_stat_ {CSD_OBJ_STAT_LOAD};

    int obj_stat__() const { return stat_.load(); }
    void obj_as_new__() { stat_ = CSD_OBJ_STAT_NEW; }
    void obj_as_load__() { stat_ = CSD_OBJ_STAT_LOAD; }
    void obj_as_save__() { stat_ = CSD_OBJ_STAT_SVAE; }
    void obj_as_dirty__() { stat_ = CSD_OBJ_STAT_DIRT; }
    void obj_as_trim__() { stat_ = CSD_OBJ_STAT_TRIM; }
    bool obj_is_new__() { return stat_.load() == CSD_OBJ_STAT_NEW; }

    std::shared_ptr<MetaStore> ms_;
    RWLock lock_;

    std::shared_ptr<CsdsClient> client_;
    std::shared_ptr<CsdsClient> make_client__();

    bool obj_readable__() const {
        int stat = stat_.load();
        if (stat != CSD_OBJ_STAT_LOAD || stat != CSD_OBJ_STAT_TRIM)
            return true;
        return false;
    }
}; // class CsdHandle

} // namespace flame

#endif // FLAME_MGR_CSDM_H