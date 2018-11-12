#include "orm/stmt.h"
#include "orm/cols.h"
#include "orm/opts.h"

#include <iostream>

using namespace std;
namespace fm = flame::orm;

/**
 * 定义一个表（Table），用于生成相应的SQL语句
 * 通过继承 flame::orm::Table（orm/stmt.h）来构建一个表， 
 * 并调用flame::orm::Table构造函数，且需要提供表名。
 * 
 * 表项的类型定义在(orm/cols.h),可以采用C++99在构造函数初始化列表去初始化表项，
 * 但是采用C++11的方式会更简洁明了。
 * 
 * 在表的类中定义方法并不是硬性要求，只有构造函数与表项是需要按要求定义的，
 * 其他一切特性都可以按C++的语义去定义。
 */
class CSDTable final : public fm::Table {
public:
    CSDTable() : fm::Table("flm_csd") {}

    fm::BigIntCol   csd_id  {this, "csd_id", fm::ColFlag::PRIMARY_KEY | fm::ColFlag::AUTO_INCREMENT};
    fm::StringCol   name    {this, "name", 64, fm::ColFlag::UNIQUE};
    fm::BigIntCol   size    {this, "size"};
    fm::BigIntCol   ctime   {this, "ctime"};
    fm::BigIntCol   io_addr {this, "io_addr"};
    fm::BigIntCol   admin_addr  {this, "admin_addr"};
    fm::SmallIntCol stat        {this, "stat"};
    fm::BigIntCol   alloced     {this, "alloced"};
    fm::BigIntCol   used        {this, "used"};
    fm::BigIntCol   write_count {this, "write_count", fm::ColFlag::NONE, 0};
    fm::BigIntCol   read_count  {this, "read_count", fm::ColFlag::NONE, 0};
    fm::BigIntCol   last_time   {this, "last_time", fm::ColFlag::NONE, 0};
    fm::BigIntCol   last_write  {this, "last_write", fm::ColFlag::NONE, 0};
    fm::BigIntCol   last_read   {this, "last_read", fm::ColFlag::NONE, 0};
    fm::BigIntCol   last_latency{this, "last_latency", fm::ColFlag::NONE, 0};
    fm::BigIntCol   last_alloc  {this, "last_alloc", fm::ColFlag::NONE, 0};
    fm::DoubleCol   load_weight {this, "load_weight", fm::ColFlag::NONE, 0.0};
    fm::DoubleCol   wear_weight {this, "wear_weight", fm::ColFlag::NONE, 0.0};
    fm::DoubleCol   total_weight{this, "total_weight", fm::ColFlag::NONE, 0.0};

    std::string create(const std::string& name_, uint64_t ctime_,
        uint64_t size_, uint64_t alloced_, uint64_t used_,
        uint64_t io_addr_, uint64_t admin_addr_, uint16_t stat_) 
    {
        fm::InsertStmt stmt;
        stmt.table(*this).column({
            name, ctime, size, alloced, used, io_addr, admin_addr, stat
        }).value({
            name_, ctime_, size_, alloced_, used_, io_addr_, admin_addr_, stat_
        });
        return stmt.to_str();
    }

    std::string remove(uint64_t csd_id_) {
        fm::DeleteStmt stmt;
        stmt.table(*this).where(csd_id == csd_id_).limit(1);
        return stmt.to_str();
    }

    fm::SelectStmt query() {
        fm::SelectStmt stmt;
        stmt.table(*this);
        return stmt;
    }
}; // class CSDTable

int main() {
    CSDTable csdtbl;
    cout << csdtbl.stmt_create() << endl;
    cout << csdtbl.stmt_drop() << endl;
    std::string query_stmt = csdtbl.query().column(csdtbl.csd_id)
                                .order_by(csdtbl.total_weight, fm::Order::DESC)
                                .limit(30).to_str();
    cout << query_stmt << endl;
    cout << csdtbl.remove(10) << endl;
}