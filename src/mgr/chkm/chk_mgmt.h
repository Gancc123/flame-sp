#ifndef FLAME_MGR_CHKM_H
#define FLAME_MGR_CHKM_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "include/meta.h"
#include "mgr/csdm/csd_mgmt.h"
#include "layout/layout.h"
#include "layout/calculator.h"

#include <cstdint>
#include <memory>
#include <map>
#include <list>

namespace flame {

class ChunkManager final {
public:
    ChunkManager(FlameContext* fct, 
        const std::shared_ptr<MetaStore>& ms, 
        const std::shared_ptr<CsdManager>& csdm,
        const std::shared_ptr<layout::ChunkLayout>& layout,
        const std::shared_ptr<layout::ChunkHealthCaculator>& chk_hlt_calor)
    : fct_(fct), ms_(ms), csdm_(csdm), layout_(layout), chk_hlt_calor_(chk_hlt_calor) {}

    /**
     * @brief 批量创建Chunk
     * 
     * @param chk_ids 指定具体需要创建的chunk id
     * @param cgn 每个CG的元素数量，保证相同CG的chunk分布在不同CSD
     * @param attr 
     * @return int 0 iff success
     */
    int create_bulk(const std::list<uint64_t>& chk_ids, int cgn, const chk_attr_t& attr);

    /**
     * @brief 以CG为单位创建Chunk
     * 
     * @param pid 主ID，用到chunk_id_t的vol_id和index，sub_id自动生成，并且从0开始
     * @param num CG中的Chk数量
     * @param attr 
     * @return int 0 iff success
     */
    int create_cg(chunk_id_t pid, int num, const chk_attr_t& attr);

    /**
     * @brief 以Volume为单位创建Chunk
     * 
     * @param pid 主ID，用到chunk_id_t的vol_id和index，sub_id自动生成，并且从0开始
     * chunk_id_t.index决定了初始的index序号
     * @param grp cg数量
     * @param cgn 每组cg内的chunk数量
     * @param attr 
     * @return int 0 iff success
     */
    int create_vol(chunk_id_t pid, int grp, int cgn, const chk_attr_t& attr);

    /**
     * @brief 以Volume为单位获取Chunk信息
     * 
     * @param chk_list 
     * @param vol_id 
     * @return int 
     */
    int info_vol(std::list<chunk_meta_t>& chk_list, const uint64_t& vol_id);

    /**
     * @brief 批量获取Chunk信息
     * 
     * @param chk_list 
     * @param chk_ids 
     * @return int 
     */
    int info_bulk(std::list<chunk_meta_t>& chk_list, const std::list<uint64_t>& chk_ids);

    // // 获取关联chunk的信息
    // int chunk_get_related(std::list<chunk_meta_t>& res_list, const uint64_t& chk_id);
    // int chunk_get_related(std::list<chunk_meta_t>& res_list, const uint64_t& vol_id, const uint16_t& index);

    /**
     * @brief 更新Chunk状态
     * 
     * @param chk_list 
     * @return int 
     */
    int update_status(const std::list<chk_push_attr_t>& chk_list);

    /**
     * @brief 更新Chunk健康信息
     * 
     * @param chk_hlt_list 
     * @return int 
     */
    int update_health(const std::list<chk_hlt_attr_t>& chk_hlt_list);

    // // 获取指定csd的limit个写热点chunk的id和写次数
    // int chunk_get_hot(const std::map<uint64_t, uint64_t>& res, const uint64_t& csd_id, const uint16_t& limit, const uint32_t& spolicy_num);

    // // 修改迁移的chunk的csd_id,分两种情况：1通知迁移 2强制迁移
    // int chunk_record_move(const chunk_move_attr_t& chk);
    
private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    std::shared_ptr<CsdManager> csdm_;
    std::shared_ptr<layout::ChunkLayout> layout_;
    std::shared_ptr<layout::ChunkHealthCaculator> chk_hlt_calor_;
}; // class ChkManager

} // namespace flame

#endif // FLAME_MGR_CHKM_H