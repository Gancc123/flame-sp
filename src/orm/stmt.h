#ifndef FLAME_ORM_STATEMENT_H
#define FLAME_ORM_STATEMENT_H

#include <string>
#include <list>
#include <initializer_list>

#include "util/str_util.h"
#include "util/clog.h"

namespace flame {
namespace orm {

class Column;

/**
 * SQL Table Base
 * Create Statement Format:
 *      CREATE TABLE IF NOT EXISTS <table_name> (
 *          <col> [, <col>]
 *          <extra>
 *      ) <footer>
 */
class Table {
public:
    Table(const std::string& table_name) : __table_name__(table_name) {}
    virtual ~Table() {}

    std::string table_name() const { return __table_name__; }

    std::string stmt_create() const;
    std::string stmt_drop() const;

    void append_extra(const std::string& str) { extra_.push_back(str); }
    void append_footer(const std::string& str) { string_append(footer_, ' ', str); }

    Table(const Table&) = default;
    Table(Table&&) = default;
    Table& operator=(const Table&) = default;
    Table& operator=(Table&&) = default;

    friend class Column;

protected:
    std::string __table_name__;

private:
    void register_col_(const Column* col) { cols_.push_back(col); }

    std::list<const Column*> cols_;
    std::list<std::string> extra_;
    std::string footer_;
}; // class Table

enum ColFlag {        // 0, 1
    NONE = 0x0,
    NOT_NULL = 0x1, // NULL, NOT NULL
    AUTO_INCREMENT = 0x2,
    UNIQUE = 0x4,
    PRIMARY_KEY = 0x8
};

/**
 * SQL Column Base
 */
class Column {
public:
    Column(Table* parent, 
        const std::string& col_name,
        const std::string& type_name,
        uint32_t flags, 
        const std::string& def = "",
        const std::string& extra = "") 
    : parent_(parent), col_name_(col_name), type_name_(type_name),
    flags_(flags), defalut_(def), extra_(extra)
    {
        parent_->register_col_(this);
    }
    virtual ~Column() {}

    std::string col_name() const { return col_name_; }
    std::string type_name() const { return type_name_; }
    std::string full_name() const { 
        return parent_ == nullptr ? 
            col_name_ : parent_->__table_name__ + "." + col_name_; 
    }
    std::string flags() const;
    std::string to_str() const;
    
    virtual Column as(const std::string& name) { return Column(name); }

    Column(const Column&) = default;
    Column(Column&&) = default;
    Column& operator=(const Column&) = default;
    Column& operator=(Column&&) = default;

protected:
    Table* parent_;
    std::string col_name_;
    std::string type_name_;
    uint32_t flags_;
    std::string defalut_;
    std::string extra_;

private:
    Column(const std::string& col_name) : parent_(nullptr), col_name_(col_name) {}
}; // class Col

class Value {
public:
    Value(int8_t t) : type_(t) {}
    virtual ~Value() {};

    Value(const Value&) = default;
    Value(Value&&) = default;
    Value& operator=(const Value&) = default;
    Value& operator=(Value&&) = default;

    enum EnumValueTypes {
        NONE = 0,   // NULL
        INT = 1,    // 64 bits signed int
        UINT = 2,   // 64 bits unsigned int
        DOUBLE = 3, // 64 bits
        STRING = 4  // string
    };

    virtual std::string to_str() const = 0;

    int8_t val_type() const { return type_; }

protected:
    int8_t type_;
}; // class ColType

/**
 * SQL Statement Base
 */
class Stmt {
public:
    Stmt() {}
    virtual ~Stmt() {}

    Stmt(const std::string& str) : stmt_(str) {}
    Stmt(const char* str) : stmt_(str) {}

    Stmt(const Stmt&) = default;
    Stmt(Stmt&&) = default;
    Stmt& operator=(const Stmt&) = default;
    Stmt& operator=(Stmt&&) = default;

    virtual std::string to_str() const { return stmt_; }
    virtual bool empty() const { return stmt_.empty(); }
    virtual void clear() { stmt_.clear(); }

protected:
    std::string stmt_;
}; // class StatementBase

/**
 * Table Column Sub Statement
 */
class ColumnStmt : public Stmt {
public:
    ColumnStmt(const Column& col) : col_(&col) {}
    virtual ~ColumnStmt() {}

    explicit ColumnStmt(const std::string& str) : Stmt(str), col_(nullptr) {}
    explicit ColumnStmt(const char* str) : Stmt(str), col_(nullptr) {}

    ColumnStmt(const ColumnStmt&) = default;
    ColumnStmt(ColumnStmt&&) = default;
    ColumnStmt& operator=(const ColumnStmt&) = default;
    ColumnStmt& operator=(ColumnStmt&&) = default;

    virtual std::string to_str() const { 
        return col_ == nullptr ? stmt_ : col_->full_name(); 
    }
    virtual bool empty() const { return stmt_.empty() && col_ == nullptr; }
    virtual void clear() { 
        stmt_.clear(); 
        col_ = nullptr; 
    }

private:
    const Column* col_;
}; // class ColumnStmt

/**
 * Table Sub Statement
 */
class TableStmt : public Stmt {
public:
    TableStmt(const Table& tbl) : table_(&tbl) {}
    virtual ~TableStmt() {}

    TableStmt(const std::string& str) : Stmt(str), table_(nullptr) {}
    TableStmt(const char* str) : Stmt(str), table_(nullptr) {}

    TableStmt(const TableStmt&) = default;
    TableStmt(TableStmt&&) = default;
    TableStmt& operator=(const TableStmt&) = default;
    TableStmt& operator=(TableStmt&&) = default;

    virtual std::string to_str() const { 
        return table_ == nullptr ? stmt_ : table_->table_name(); 
    }
    virtual bool empty() const { return stmt_.empty() && table_ == nullptr; }
    virtual void clear() { 
        stmt_.clear(); 
        table_ = nullptr; 
    }

private:
    const Table* table_;
}; // class TableStmt

/**
 * Value Sub Statement
 */
class ValueStmt : public Stmt {
public:
    ValueStmt(const Value& val) : val_(&val) {}
    virtual ~ValueStmt() {}

    ValueStmt(const std::string& str) : Stmt(string_encode_box(str, '\'')), val_(nullptr) {}
    ValueStmt(const char* str) : Stmt(string_encode_box(str, '\'')), val_(nullptr) {}
    ValueStmt(char val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(unsigned char val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(short val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(unsigned short val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(int val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(unsigned int val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(long val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(unsigned long val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(long long val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(unsigned long long val) : Stmt(convert2string(val)), val_(nullptr) {}
    ValueStmt(double val) : Stmt(convert2string(val)), val_(nullptr) {}

    ValueStmt(const ValueStmt&) = default;
    ValueStmt(ValueStmt&&) = default;
    ValueStmt& operator=(const ValueStmt&) = default;
    ValueStmt& operator=(ValueStmt&&) = default;

    virtual std::string to_str() const { 
        return val_ == nullptr ? stmt_ : val_->to_str(); 
    }
    virtual bool empty() const { return stmt_.empty() && val_ == nullptr; }
    virtual void clear() { 
        stmt_.clear(); 
        val_ = nullptr; 
    }

private:
    const Value* val_;
}; // class ColStmt

/**
 * Condition Sub Statment
 */
class CondStmt : public Stmt {
public:
    virtual ~CondStmt() {}

    explicit CondStmt(const std::string& str) : Stmt(str) {}
    explicit CondStmt(const char* str) : Stmt(str) {}
    explicit CondStmt(const Stmt& stmt) : Stmt(stmt) {}
    explicit CondStmt(Stmt&& stmt) : Stmt(stmt) {}

    CondStmt(const CondStmt&) = default;
    CondStmt(CondStmt&&) = default;
    CondStmt& operator=(const CondStmt&) = default;
    CondStmt& operator=(CondStmt&&) = default;

    CondStmt& and_(const CondStmt& cond);

    CondStmt& or_(const CondStmt& cond);

    virtual std::string to_str() const override { return stmt_; }
    virtual bool empty() const override { return stmt_.empty(); }
    virtual void clear() override { Stmt::clear(); }

}; // class CondStmt

/**
 * Expression Sub Statment
 */
class ExprStmt : public Stmt {
public:
    virtual ~ExprStmt() {}
 
    explicit ExprStmt(const std::string& str) : Stmt(str) {}
    explicit ExprStmt(const char* str) : Stmt(str) {}
    explicit ExprStmt(const Stmt& stmt) : Stmt(stmt) {}
    explicit ExprStmt(Stmt&& stmt) : Stmt(stmt) {}

    ExprStmt(const ValueStmt& stmt) : Stmt(stmt) {}
    ExprStmt(const ColumnStmt& stmt) : Stmt(stmt.to_str()) {}

    ExprStmt(const ExprStmt&) = default;
    ExprStmt(ExprStmt&&) = default;
    ExprStmt& operator=(const ExprStmt&) = default;
    ExprStmt& operator=(ExprStmt&&) = default;

    virtual std::string to_str() const override { return stmt_; }
    virtual bool empty() const override { return stmt_.empty(); }
    virtual void clear() override { Stmt::clear(); }

}; // class ExprStmt

/**
 * Assignment Sub Statment
 */
class AssignStmt : public Stmt {
public:
    virtual ~AssignStmt() {}

    explicit AssignStmt(const std::string& str) : Stmt(str) {}
    explicit AssignStmt(const char* str) : Stmt(str) {}
    explicit AssignStmt(const Stmt& stmt) : Stmt(stmt) {}
    explicit AssignStmt(Stmt&& stmt) : Stmt(stmt) {}

    AssignStmt(const AssignStmt&) = default;
    AssignStmt(AssignStmt&&) = default;
    AssignStmt& operator=(const AssignStmt&) = default;
    AssignStmt& operator=(AssignStmt&&) = default;

    virtual std::string to_str() const override { return stmt_; }
    virtual bool empty() const override { return stmt_.empty(); }
    virtual void clear() override { Stmt::clear(); }

}; // class ExprStmt

enum Order {
    ASC = 0,
    DESC = 1
};

/**
 * Select Statement
 */
class SelectStmt : public Stmt {
public:
    SelectStmt() : offset_(0), limit_(0) {}
    virtual ~SelectStmt() {}

    SelectStmt(const SelectStmt&) = default;
    SelectStmt(SelectStmt&&) = default;
    SelectStmt& operator=(const SelectStmt&) = default;
    SelectStmt& operator=(SelectStmt&&) = default;

    SelectStmt& column(const ColumnStmt& col) {
        string_append(cols_, ", ", col.to_str());
        return *this;
    }

    SelectStmt& column(const std::initializer_list<ColumnStmt>& cols) {
        for (auto it = cols.begin(); it != cols.end(); it++)
            column(*it);
        return *this;
    }

    SelectStmt& table(const TableStmt& tbl) { 
        string_append(tables_, ", ", tbl.to_str());
        return *this;
    }

    SelectStmt& table(const std::initializer_list<TableStmt>& tbls) {
        for (auto it = tbls.begin(); it != tbls.end(); it++)
            table(*it);
        return *this;
    }

    SelectStmt& join(const TableStmt& tbl) { return join_("JOIN", tbl); }
    SelectStmt& join(const TableStmt& tbl, const CondStmt& on) {
        return join_("JOIN", tbl, on);
    }

    SelectStmt& inner_join(const TableStmt& tbl) { return join_("INNER JOIN", tbl); }
    SelectStmt& inner_join(const TableStmt& tbl, const CondStmt& on) {
        return join_("INNER JOIN", tbl, on);
    }

    SelectStmt& left_join(const TableStmt& tbl) { return join_("LEFT JOIN", tbl); }
    SelectStmt& left_join(const TableStmt& tbl, const CondStmt& on) {
        return join_("LEFT JOIN", tbl, on);
    }

    SelectStmt& right_join(const TableStmt& tbl) { return join_("RIGHT JOIN", tbl); }
    SelectStmt& right_join(const TableStmt& tbl, const CondStmt& on) {
        return join_("RIGHT JOIN", tbl, on);
    }

    SelectStmt& full_join(const TableStmt& tbl) { return join_("FULL JOIN", tbl); }
    SelectStmt& full_join(const TableStmt& tbl, const CondStmt& on) {
        return join_("FULL JOIN", tbl, on);
    }

    SelectStmt& where(const CondStmt& cond);

    SelectStmt& where(const std::initializer_list<CondStmt>& conds) {
        for (auto it = conds.begin(); it != conds.end(); it++)
            where(*it);
        return *this;
    }

    SelectStmt& group_by(const ColumnStmt& col) {
        string_append(group_by_, ", ", col.to_str());
        return *this;
    }

    SelectStmt& group_by(const std::initializer_list<ColumnStmt>& cols) {
        for (auto it = cols.begin(); it != cols.end(); it++)
            group_by(*it);
        return *this;
    }

    SelectStmt& having(const CondStmt& cond) {
        string_append(having_, " AND ", cond.to_str());
        return *this;
    }

    SelectStmt& having(const std::initializer_list<CondStmt>& conds) {
        for (auto it = conds.begin(); it != conds.end(); it++) 
            having(*it);
        return *this;
    }

    SelectStmt& order_by(const ColumnStmt& col, int order = Order::ASC);

    SelectStmt& offset(size_t n) {
        offset_ = n;
        return *this;
    }

    SelectStmt& limit(size_t n) { 
        limit_ = n;
        return *this;
    }

    virtual std::string to_str() const override;

    virtual bool empty() const override;

    virtual void clear() override;

private:
    SelectStmt& join_(const std::string& jt, const TableStmt& tbl);
    SelectStmt& join_(const std::string& jt, const TableStmt& tbl, const CondStmt& on);

    std::string cols_;
    std::string tables_;
    std::string joins_;
    std::string conditions_;
    std::string group_by_;
    std::string having_;
    std::string order_by_;
    uint64_t offset_;
    uint64_t limit_;
}; // class SelectStmt

/**
 * Insert Statement
 */
class InsertStmt : public Stmt {
public:
    InsertStmt() {}
    virtual ~InsertStmt() {}

    InsertStmt(const InsertStmt&) = default;
    InsertStmt(InsertStmt&&) = default;
    InsertStmt& operator=(const InsertStmt&) = default;
    InsertStmt& operator=(InsertStmt&&) = default;

    InsertStmt& table(const TableStmt& tbl) {
        table_ = tbl.to_str();
        return *this;
    }

    InsertStmt& column(const ColumnStmt& col) { 
        string_append(partitions_, ", ", col.to_str());
        return *this;
    }

    InsertStmt& column(const std::initializer_list<ColumnStmt>& cols) {
        for (auto it = cols.begin(); it != cols.end(); it++)
            column(*it);
        return *this;
    }

    InsertStmt& value(const ValueStmt& val) { 
        string_append(values_, ", ", val.to_str());
        return *this;
    }

    InsertStmt& value(const std::initializer_list<ValueStmt>& vals) {
        for (auto it = vals.begin(); it != vals.end(); it++)
            value(*it);
        return *this;
    }

    InsertStmt& select(const SelectStmt& stmt) {
        select_ = stmt.to_str();
        return *this;
    }

    virtual std::string to_str() const override;

    virtual bool empty() const override;

    virtual void clear() override;

private:
    std::string table_;
    std::string partitions_;
    std::string values_;
    std::string select_;
}; // class InsertStmt


/**
 * Update Statement
 */
class UpdateStmt : public Stmt {
public:
    UpdateStmt() : limit_(0) {}
    virtual ~UpdateStmt() {}

    UpdateStmt(const UpdateStmt&) = default;
    UpdateStmt(UpdateStmt&&) = default;
    UpdateStmt& operator=(const UpdateStmt&) = default;
    UpdateStmt& operator=(UpdateStmt&&) = default;

    UpdateStmt& table(const TableStmt& tbl) { 
        string_append(tables_, ", ", tbl.to_str());
        return *this;
    }

    UpdateStmt& table(const std::initializer_list<TableStmt>& tbls) {
        for (auto it = tbls.begin(); it != tbls.end(); it++)
            table(*it);
        return *this;
    }

    UpdateStmt& assign(const AssignStmt& stmt) { 
        string_append(assignment_, ", ", stmt.to_str());
        return *this;
    }

    UpdateStmt& assign(const std::initializer_list<AssignStmt>& stmts) {
        for (auto it = stmts.begin(); it != stmts.end(); it++)
            assign(*it);
        return *this;
    }

    UpdateStmt& where(const CondStmt& cond);

    UpdateStmt& where(const std::initializer_list<CondStmt>& conds) {
        for (auto it = conds.begin(); it != conds.end(); it++) 
            where(*it);
        return *this;
    }

    UpdateStmt& order_by(const ColumnStmt& col, int order = Order::ASC);

    UpdateStmt& limit(size_t n) { 
        limit_ = n;
        return *this;
    }

    virtual std::string to_str() const override;

    virtual bool empty() const override;

    virtual void clear() override;

private:
    std::string tables_;
    std::string assignment_;
    std::string conditions_;
    std::string order_by_;
    uint64_t limit_;
}; // class UpdateStmt

/**
 * Delete Statement
 */
class DeleteStmt : public Stmt {
public:
    DeleteStmt() {}
    virtual ~DeleteStmt() {}

    DeleteStmt(const DeleteStmt&) = default;
    DeleteStmt(DeleteStmt&&) = default;
    DeleteStmt& operator=(const DeleteStmt&) = default;
    DeleteStmt& operator=(DeleteStmt&&) = default;

    DeleteStmt& table(const TableStmt& tbl) { 
        table_ = tbl.to_str();
        return *this;
    }

    DeleteStmt& where(const CondStmt& cond);

    DeleteStmt& where(const std::initializer_list<CondStmt>& conds) {
        for (auto it = conds.begin(); it != conds.end(); it++)
            where(*it);
        return *this;
    }

    DeleteStmt& order_by(const ColumnStmt& col, int order = Order::ASC);

    DeleteStmt& limit(size_t n) { 
        limit_ = n;
        return *this;
    }

    virtual std::string to_str() const override;

    virtual bool empty() const override;

    virtual void clear() override;

private:
    std::string table_;
    std::string conditions_;
    std::string order_by_;
    uint64_t limit_;
}; // class DeleteStmt

class MultiInsertStmt : public Stmt {
public:
    MultiInsertStmt() {}
    virtual ~MultiInsertStmt() {}

    MultiInsertStmt(const MultiInsertStmt&) = default;
    MultiInsertStmt(MultiInsertStmt&&) = default;
    MultiInsertStmt& operator=(const MultiInsertStmt&) = default;
    MultiInsertStmt& operator=(MultiInsertStmt&&) = default;

    MultiInsertStmt& table(const TableStmt& tbl) {
        table_ = tbl.to_str();
        return *this;
    }

    MultiInsertStmt& column(const std::initializer_list<ColumnStmt>& cols) {
        for (auto it = cols.begin(); it != cols.end(); it++)
            string_append(partitions_, ", ", it->to_str());
        return *this;
    }

    MultiInsertStmt& value(const std::initializer_list<ValueStmt>& vals);

    virtual std::string to_str() const override;

    virtual bool empty() const override;

    virtual void clear() override;

private:
    std::string table_;
    std::string partitions_;
    std::list<std::string> value_list_;
}; // class InsertStmt

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_STATEMENT_H