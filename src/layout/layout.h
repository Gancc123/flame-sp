#ifndef FLAME_LAYOUT_H
#define FLAME_LAYOUT_H

#include <cstdint>
#include <memory>
#include <list>
#include "include/retcode.h"
#include "mgr/csdm/csd_mgmt.h"
#include "work/timer_work.h"

namespace flame {

class ChunkLayout {
public:
    virtual ~ChunkLayout() {}

    /**
     * @brief 布局方案
     * 
     * @return int 布局方案编号
     */
    virtual int type() = 0;

    /**
     * @brief 选择CSD
     * 选择num个不同的CSD，并确保每个CSD都能够容纳chk_sz大小的Chunk
     * @param csd_ids 返回值，一组CSD ID， 数量与 num 相等
     * @param num 需要的CSD数量
     * @param chk_sz Chunk大小
     * @return int 0 iff success
     */
    virtual int select(std::list<uint64_t>& csd_ids, int num, uint64_t chk_sz) = 0;

    /**
     * @brief 选择多组CSD
     * 同事选择多组CSD，规则与select()一样
     * @param csd_ids 返回 
     * @param grp CSD组数 
     * @param cgn 每个CG的chunk数量 
     * @param chk_sz 
     * @return int 0 iff success
     */
    virtual int select_bulk(std::list<uint64_t>& csd_ids, int grp, int cgn, uint64_t chk_sz) = 0;

protected:
    ChunkLayout(const std::shared_ptr<CsdManager>& csdm) : csdm_(csdm) {}

    std::shared_ptr<CsdManager> csdm_;
}; // class ChunkLayout

} // namespace flame

#endif // FLAME_LAYOUT_H