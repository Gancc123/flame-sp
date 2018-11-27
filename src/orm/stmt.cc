#include "stmt.h"
#include "cols.h"

namespace flame {
namespace orm {

std::string Table::stmt_create() const {
    std::string cont;
    for (auto it = cols_.begin(); it != cols_.end(); it++) 
        string_append(cont, ", ", (*it)->to_str());
    for (auto it = extra_.begin(); it != extra_.end(); it++)
        string_append(cont, ", ", *it);

    std::string stmt;
    stmt += "CREATE TABLE IF NOT EXISTS ";
    stmt += __table_name__;
    stmt += "(";
    stmt += cont;
    stmt += ")";
    stmt += footer_;
    return stmt;
}

std::string Table::stmt_drop() const {
    std::string stmt;
    stmt += "DROP TABLE ";
    stmt += __table_name__;
    return stmt;
}

std::string Column::flags() const {
    std::string str;
    if (flags_ & ColFlag::NOT_NULL)
        string_append(str, ' ', "NOT NULL");
    else if (!(flags_ & PRIMARY_KEY))
        string_append(str, ' ', "NULL");
    if (flags_ & ColFlag::AUTO_INCREMENT)
        string_append(str, ' ', "AUTO_INCREMENT");
    if (flags_ & ColFlag::UNIQUE)
        string_append(str, ' ', "UNIQUE");
    if (flags_ & ColFlag::PRIMARY_KEY)
        string_append(str, ' ', "PRIMARY KEY");
    return str;
}

std::string Column::to_str() const {
    std::string stmt;
    string_append(stmt, ' ', string_concat({"`", col_name_, "`"}));
    string_append(stmt, ' ', type_name_);
    if (flags_ != 0)
        string_append(stmt, ' ', flags());
    if (!defalut_.empty()) {
        string_append(stmt, ' ', "DEFAULT");
        string_append(stmt, ' ', defalut_);
    }
    if (!extra_.empty())
        string_append(stmt, ' ', extra_);
    return stmt;
}

CondStmt& CondStmt::and_(const CondStmt& cond) {
    std::string stmt = "(";
    stmt += cond.to_str();
    stmt += ")";
    string_append(stmt_, " AND ", stmt);
    return *this;
}

CondStmt& CondStmt::or_(const CondStmt& cond) {
    std::string stmt = "(";
    stmt += cond.to_str();
    stmt += ")";
    string_append(stmt_, " OR ", stmt);
    return *this;
}

SelectStmt& SelectStmt::where(const CondStmt& cond) {
    std::string tail = "(";
    tail += cond.to_str();
    tail += ")";
    string_append(conditions_, " AND ", tail);
    return *this;
}

SelectStmt& SelectStmt::order_by(const ColumnStmt& col, int order) {
    std::string unit = col.to_str();
    if (order == Order::ASC) 
        unit.append(" ASC");
    else if (order == Order::DESC)
        unit.append(" DESC");
    string_append(order_by_, ", ", unit);
    return *this;
}

std::string SelectStmt::to_str() const {
    std::string stmt;
    stmt += "SELECT ";
    if (cols_.empty()) {
        stmt += " * ";
    } else {
        stmt += cols_;
    }
    stmt += " FROM ";
    stmt += tables_;
    if (!joins_.empty()) {
        stmt += " ";
        stmt += joins_;
    }
    if (!conditions_.empty()) {
        stmt += " WHERE ";
        stmt += conditions_;
    }
    if (!group_by_.empty()) {
        stmt += " GROUP BY ";
        stmt += group_by_;
    }
    if (!having_.empty()) {
        stmt += " HAVING ";
        stmt += having_;
    }
    if (!order_by_.empty()) {
        stmt += " ORDER BY ";
        stmt += order_by_;
    }
    if (limit_ != 0) {
        stmt += " LIMIT ";
        stmt += convert2string(limit_);
    }
    if (offset_ != 0) {
        stmt += " OFFSET ";
        stmt += convert2string(offset_);
    }
    return stmt;
}

bool SelectStmt::empty() const {
    return cols_.empty()
        && tables_.empty()
        && joins_.empty()
        && conditions_.empty()
        && group_by_.empty()
        && having_.empty()
        && order_by_.empty()
        && offset_ == 0
        && limit_ == 0;
}

void SelectStmt::clear() {
    Stmt::clear();
    cols_.clear();
    tables_.clear();
    joins_.clear();
    conditions_.clear();
    group_by_.clear();
    having_.clear();
    order_by_.clear();
    offset_ = 0;
    limit_ = 0;
}

SelectStmt& SelectStmt::join_(const std::string& jt, const TableStmt& tbl) {
    if (!tbl.empty()) {
        string_append(joins_, ' ', jt);
        string_append(joins_, ' ', tbl.to_str());
    }
    return *this;
}

SelectStmt& SelectStmt::join_(const std::string& jt, const TableStmt& tbl, const CondStmt& on) {
    if (!tbl.empty()) {
        string_append(joins_, ' ', jt);
        string_append(joins_, ' ', tbl.to_str());
    }
    if (!on.empty()) {
        string_append(joins_, ' ', "ON");
        string_append(joins_, ' ', on.to_str());
    }
    return *this;
}

std::string InsertStmt::to_str() const {
    std::string stmt;
    stmt += "INSERT INTO ";
    stmt += table_;
    if (!partitions_.empty()) {
        stmt += "(";
        stmt += partitions_;
        stmt += ")";
    }
    if (!values_.empty()) {
        stmt += " VALUES (";
        stmt += values_;
        stmt += ")";
    }
    if (!select_.empty()) {
        stmt += " ";
        stmt += select_;
    }
    return stmt;
}

bool InsertStmt::empty() const {
    return table_.empty()
        && partitions_.empty()
        && values_.empty()
        && select_.empty();
}

void InsertStmt::clear() {
    Stmt::clear();
    table_.clear();
    partitions_.clear();
    values_.clear();
    select_.clear();
}

UpdateStmt& UpdateStmt::where(const CondStmt& cond) {
    std::string tail = "(";
    tail += cond.to_str();
    tail += ")";
    string_append(conditions_, " AND ", tail);
    return *this;
}

UpdateStmt& UpdateStmt::order_by(const ColumnStmt& col, int order) {
    std::string unit = col.to_str();
    if (order == Order::ASC) 
        unit.append(" ASC");
    else if (order == Order::DESC)
        unit.append(" DESC");
    string_append(order_by_, ", ", unit);
    return *this;
}

bool UpdateStmt::empty() const {
    return tables_.empty()
        && assignment_.empty()
        && conditions_.empty()
        && order_by_.empty()
        && limit_ == 0;
}

std::string UpdateStmt::to_str() const {
    std::string stmt;
    stmt += "UPDATE ";
    stmt += tables_;
    stmt += " SET ";
    stmt += assignment_;
    if (!order_by_.empty()) {
        stmt += " ORDER BY ";
        stmt += order_by_;
    }
    if (limit_ != 0) {
        stmt += " LIMIT ";
        stmt += convert2string(limit_);
    }
    return stmt;
}

void UpdateStmt::clear() {
    Stmt::clear();
    tables_.clear();
    assignment_.clear();
    conditions_.clear();
    order_by_.clear();
    limit_ = 0;
}

DeleteStmt& DeleteStmt::where(const CondStmt& cond) {
    std::string tail = "(";
    tail += cond.to_str();
    tail += ")";
    string_append(conditions_, " AND ", tail);
    return *this;
}

DeleteStmt& DeleteStmt::order_by(const ColumnStmt& col, int order) {
    std::string unit = col.to_str();
    if (order == Order::ASC) 
        unit.append(" ASC");
    else if (order == Order::DESC)
        unit.append(" DESC");
    string_append(order_by_, ", ", unit);
    return *this;
}

std::string DeleteStmt::to_str() const {
    std::string stmt;
    stmt += "DELETE ";
    stmt += table_;
    if (!conditions_.empty()) {
        stmt += " WHERE ";
        stmt += conditions_;
    }
    if (!order_by_.empty()) {
        stmt += " ORDER BY ";
        stmt += order_by_;
    }
    if (limit_ != 0) {
        stmt += " LIMIT ";
        stmt += convert2string(limit_);
    }
    return stmt;
}

bool DeleteStmt::empty() const {
    return table_.empty()
        && conditions_.empty()
        && order_by_.empty()
        && limit_ == 0;
}

void DeleteStmt::clear() {
    Stmt::clear();
    table_.clear();
    conditions_.clear();
    order_by_.clear();
    limit_ = 0;
}

MultiInsertStmt& MultiInsertStmt::value(const std::initializer_list<ValueStmt>& vals) {
    std::string value;
    for (auto it = vals.begin(); it != vals.end(); it++)
        string_append(value, ", ", it->to_str());
    value_list_.push_back(value);
    return *this;
}

std::string MultiInsertStmt::to_str() const {
    std::string stmt;
    stmt += "INSERT INTO ";
    stmt += table_;
    if (!partitions_.empty()) {
        stmt += "(";
        stmt += partitions_;
        stmt += ")";
    }
    if (!value_list_.empty()) {
        stmt += " VALUES ";
        int count = value_list_.size();
        int idx = 0;
        for (auto it = value_list_.begin(); it != value_list_.end(); it++) {
            idx++;
            stmt += "(";
            stmt += *it;
            if (idx == count) {
                stmt += ")";
            } else {
                stmt += "), ";
            }
        }
    }
    return stmt;
}

bool MultiInsertStmt::empty() const {
    return table_.empty()
        && partitions_.empty()
        && value_list_.empty();
}

void MultiInsertStmt::clear() {
    Stmt::clear();
    table_.clear();
    partitions_.clear();
    value_list_.clear();
}

} // namespace orm
} // namespace flame