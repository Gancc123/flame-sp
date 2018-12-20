#ifndef FLAME_METASTORE_SQLMS_MODELS_H
#define FLAME_METASTORE_SQLMS_MODELS_H

#include "orm/orm.h"

#include <memory>

namespace flame {

class ClusterModel final : public orm::DBTable {
public:
    ClusterModel(const std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "cluster") { auto_create(); }

    // clt_id set with utime::now()
    orm::BigIntCol  clt_id  {this, "clt_id", orm::ColFlag::PRIMARY_KEY};
    orm::StringCol  name    {this, "name", 64};
    orm::IntCol     mgrs    {this, "mgrs"};
    orm::IntCol     csds    {this, "csds"};
    orm::BigIntCol  ctime   {this, "ctime"};
    orm::BigIntCol  size    {this, "size"};
    orm::BigIntCol  alloced {this, "alloced"};
    orm::BigIntCol  used    {this, "used"};
}; // class ClusterModel

class VolumeGroupModel final : public orm::DBTable {
public:
    VolumeGroupModel(std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "volume_group") { auto_create(); }

    orm::BigIntCol   vg_id   {this, "vg_id", orm::ColFlag::PRIMARY_KEY | orm::ColFlag::AUTO_INCREMENT};
    orm::StringCol   name    {this, "name", 64};
    orm::BigIntCol   ctime   {this, "ctime"};   // 创建时间
    orm::IntCol      volumes {this, "volumes", orm::ColFlag::NONE, 0};   // volume个数
    orm::BigIntCol   size    {this, "size", orm::ColFlag::NONE, 0}; // 可视总大小（B）
    orm::BigIntCol   alloced {this, "alloced", orm::ColFlag::NONE, 0};  // 所需要占用的空间（B）
    orm::BigIntCol   used    {this, "used"};    // 实际已占用空间（B）
}; // class VolumeGroupModel

class VolumeModel final : public orm::DBTable {
public:
    VolumeModel(std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "volume") { auto_create(); }
    
    orm::BigIntCol  vol_id  {this, "vol_id", orm::ColFlag::PRIMARY_KEY | orm::ColFlag::AUTO_INCREMENT};    // Volume的唯一标识
    orm::BigIntCol  vg_id   {this, "vg_id"};    // Group标识
    orm::StringCol  name    {this, "name", 64}; // Volume名称
    orm::BigIntCol  ctime   {this, "ctime"};    // 创建时间
    orm::BigIntCol  chk_sz  {this, "chk_sz"};   // Chunk Size
    orm::BigIntCol  size    {this, "size"};     // 可视大小（B）
    orm::BigIntCol  alloced {this, "alloced"};  // 所需分配的空间（B）
    orm::BigIntCol  used    {this, "used"};     // 实际已使用空间（B）
    orm::IntCol     flags   {this, "flags"};    // 标签
    orm::IntCol     spolicy {this, "spolicy"};  // 存储策略
    orm::IntCol     chunks  {this, "chunks"};   // Chunk数量
}; // class VolumeModel

class ChunkModel final : public orm::DBTable {
public:
    ChunkModel(std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "chunk") { auto_create(); }
    
    orm::BigIntCol  chk_id  {this, "chk_id", orm::ColFlag::PRIMARY_KEY}; // Chunk ID
    orm::BigIntCol  vol_id  {this, "vol_id"};   // Volume ID
    orm::IntCol     index   {this, "index"};    // Index in Same Volume
    orm::IntCol     stat    {this, "stat"};     // CSD状态
    orm::IntCol     spolicy {this, "spolicy"};
    orm::BigIntCol  ctime   {this, "ctime"};    // 创建时间
    orm::BigIntCol  primary {this, "primary"};
    orm::BigIntCol  size    {this, "size"};     // 可视大小（B）
    orm::BigIntCol  csd_id  {this, "csd_id"};   // 当前所在CSD ID
    orm::BigIntCol  csd_mtime   {this, "csd_mtime"};    // 当前所在CSD的映射时间
    orm::BigIntCol  dst_id  {this, "dst_id"};   // 目标CSD ID（迁移）
    orm::BigIntCol  dst_ctime   {this, "dst_ctime"};//目标CSD 映射时间（迁移的开始时间）
}; // class ChunkModel

class ChunkHealthModel final : public orm::DBTable {
public:
    ChunkHealthModel(std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "chunk_health") { auto_create(); }
    
    orm::BigIntCol  chk_id      {this, "chk_id", orm::ColFlag::PRIMARY_KEY}; // Chunk ID
    orm::IntCol     stat        {this, "stat"};
    orm::BigIntCol  size        {this, "size"}; // 可视大小（B）
    orm::BigIntCol  used        {this, "used"}; // 实际已使用的空间（B
    orm::BigIntCol  csd_used    {this, "csd_used"}; // 有效已使用的空间，针对迁移场景（B）
    orm::BigIntCol  dst_used    {this, "dst_used"};
    orm::BigIntCol  write_count {this, "write_count", orm::ColFlag::NONE, 0};  // 写计数（以page大小为单位，4KB）
    orm::BigIntCol  read_count  {this, "read_count", orm::ColFlag::NONE, 0};   // 读计数（以page大小为单位，4KB）
    orm::BigIntCol  last_time   {this, "last_time", orm::ColFlag::NONE, 0};    // 上一次监控记录时间
    orm::BigIntCol  last_write  {this, "last_write", orm::ColFlag::NONE, 0};   // 上一个监控周期写次数
    orm::BigIntCol  last_read   {this, "last_read", orm::ColFlag::NONE, 0};    // 上一个监控周期读次数
    orm::BigIntCol  last_latency {this, "last_latency", orm::ColFlag::NONE, 0};    // 上一个监控周期平均延迟（ns）
    orm::BigIntCol  last_alloc  {this, "last_alloc", orm::ColFlag::NONE, 0};   // 上一个监控周期新分配的空间（B）
    orm::DoubleCol  load_weight {this, "load_weight", orm::ColFlag::NONE, 0};  // 负载权值
    orm::DoubleCol  wear_weight {this, "wear_weight", orm::ColFlag::NONE, 0};  // 磨损权值
    orm::DoubleCol  total_weight {this, "total_weight", orm::ColFlag::NONE, 0};    // 综合权值
}; // class ChunkHealthModel

class CsdModel final : public orm::DBTable {
public:
    CsdModel(std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "csd") { auto_create(); } 

    orm::BigIntCol  csd_id  {this, "csd_id", orm::ColFlag::PRIMARY_KEY}; // CSD ID
    orm::StringCol  name    {this, "name", 64, orm::ColFlag::UNIQUE};    // 名称
    orm::BigIntCol  size    {this, "size"}; // 可视大小，这里指SSD的容量（B）
    orm::BigIntCol  ctime   {this, "ctime"};    // 创建时间
    orm::BigIntCol  io_addr {this, "io_addr"};  // 数据平面/IO地址
    orm::BigIntCol  admin_addr  {this, "admin_addr"};   // 控制平面/ADMIN地址
    orm::IntCol     stat    {this, "stat"}; // 节点状态：DOWN/PAUSE/ACTIVE
    orm::BigIntCol  latime  {this, "latime"};
}; // class CsdModel

class CsdHealthModel final : public orm::DBTable {
public:
    CsdHealthModel(std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "csd_health") { auto_create(); } 

    orm::BigIntCol  csd_id      {this, "csd_id", orm::ColFlag::PRIMARY_KEY};    // CSD ID
    orm::IntCol     stat        {this, "stat"};
    orm::BigIntCol  size        {this, "size"};
    orm::BigIntCol  alloced     {this, "alloced"};  // 需要分配的空间（B）
    orm::BigIntCol  used        {this, "used"}; // 实际已使用的空间（B）
    orm::BigIntCol  write_count {this, "write_count", orm::ColFlag::NONE, 0};   // 写计数（以page大小为单位，4KB）
    orm::BigIntCol  read_count  {this, "read_count", orm::ColFlag::NONE, 0};    // 读计数（以page大小为单位，4KB）
    orm::BigIntCol  last_time   {this, "last_time", orm::ColFlag::NONE, 0}; // 最后一次监控记录时间
    orm::BigIntCol  last_write  {this, "last_write", orm::ColFlag::NONE, 0};    // 上一个监控周期写次数
    orm::BigIntCol  last_read   {this, "last_read", orm::ColFlag::NONE, 0}; // 上一个监控周期读次数
    orm::BigIntCol  last_latency{this, "last_latency", orm::ColFlag::NONE, 0};  // 上一个监控周期平均延迟（ns）
    orm::BigIntCol  last_alloc  {this, "last_alloc", orm::ColFlag::NONE, 0};    // 上一个监控周期新分配的空间（B）
    orm::DoubleCol  load_weight {this, "load_weight", orm::ColFlag::NONE, 0.0}; // 负载权值
    orm::DoubleCol  wear_weight {this, "wear_weight", orm::ColFlag::NONE, 0.0}; // 磨损权值
    orm::DoubleCol  total_weight{this, "total_weight", orm::ColFlag::NONE, 0.0};    // 综合权值
}; // class CsdHealthModel

class GatewayModel final : public orm::DBTable {
public:
    GatewayModel(std::shared_ptr<orm::DBEngine>& engine) 
    : orm::DBTable(engine, "gateway") { auto_create(); }   

    orm::BigIntCol  gw_id       {this, "gw_id", orm::ColFlag::PRIMARY_KEY}; // GateWay ID
    orm::BigIntCol  admin_addr  {this, "admin_addr", orm::ColFlag::UNIQUE}; // 控制平面地址，用于接收主动推送的信息
    orm::BigIntCol  ltime       {this, "ltime"};    // 连接时间
    orm::BigIntCol  atime       {this, "atime"};    // MGR最后一次推送信息成功时间或者GW最后一次主动发送消息给MGR的时间
}; // class GatewayModel

} // namespace flame

#endif // FLAME_METASTORE_SQLMS_MODELS_H