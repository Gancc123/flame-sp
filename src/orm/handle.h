#ifndef FLAME_ORM_HANDLE_H
#define FLAME_ORM_HANDLE_H

#include "driver.h"
#include "stmt.h"
#include "engine.h"

#include <initializer_list>

namespace flame {
namespace orm {

/**
 * Handle
 * A rumtime pointor to Stmt
 */
class Handle {
public:
    virtual ~Handle() {}

    virtual std::shared_ptr<Result> exec() = 0;

    Handle(const Handle&) = default;
    Handle(Handle&&) = default;
    Handle& operator=(const Handle&) = default;
    Handle& operator=(Handle&&) = default;

protected:
    Handle(const std::shared_ptr<DBEngine>& engine) : engine_(engine) {}

    std::shared_ptr<DBEngine> engine_;
}; // class Handle

class SelectHandle final : public Handle {
public:
    SelectHandle(const std::shared_ptr<DBEngine>& engine) : Handle(engine) {}
    ~SelectHandle() {}

    std::shared_ptr<Result> exec() override { return engine_->execute(stmt); }

    SelectHandle& column(const ColumnStmt& col) { stmt.column(col); return *this; }
    SelectHandle& column(const std::initializer_list<ColumnStmt>& cols) {
        stmt.column(cols);
        return *this;
    }

    SelectHandle& table(const TableStmt& tbl) { stmt.table(tbl); return *this; }
    SelectHandle& table(const std::initializer_list<TableStmt>& tbls) {
        stmt.table(tbls);
        return *this;
    }

    SelectHandle& join(const TableStmt& tbl) { stmt.join(tbl); return *this; }
    SelectHandle& join(const TableStmt& tbl, const CondStmt& on) {
        stmt.join(tbl, on);
        return *this;
    }

    SelectHandle& inner_join(const TableStmt& tbl) { stmt.inner_join(tbl); return *this; }
    SelectHandle& inner_join(const TableStmt& tbl, const CondStmt& on) {
        stmt.inner_join(tbl, on);
        return *this;
    }

    SelectHandle& left_join(const TableStmt& tbl) { stmt.left_join(tbl); return *this; }
    SelectHandle& left_join(const TableStmt& tbl, const CondStmt& on) {
        stmt.left_join(tbl, on);
        return *this;
    }

    SelectHandle& right_join(const TableStmt& tbl) { stmt.right_join(tbl); return *this; }
    SelectHandle& right_join(const TableStmt& tbl, const CondStmt& on) {
        stmt.right_join(tbl, on);
        return *this;
    }

    SelectHandle& full_join(const TableStmt& tbl) { stmt.full_join(tbl); return *this; }
    SelectHandle& full_join(const TableStmt& tbl, const CondStmt& on) {
        stmt.full_join(tbl, on);
        return *this;
    }

    SelectHandle& where(const CondStmt& cond) { stmt.where(cond); return *this; }
    SelectHandle& where(const std::initializer_list<CondStmt>& conds) {
        stmt.where(conds);
        return *this;
    }

    SelectHandle& group_by(const ColumnStmt& col) { stmt.group_by(col); return *this; }
    SelectHandle& group_by(const std::initializer_list<ColumnStmt>& cols) {
        stmt.group_by(cols);
        return *this;
    }

    SelectHandle& having(const CondStmt& cond) { stmt.having(cond); return *this; }
    SelectHandle& having(const std::initializer_list<CondStmt>& conds) {
        stmt.having(conds);
        return *this;
    }

    SelectHandle& order_by(const ColumnStmt& col, int order = Order::ASC) {
        stmt.order_by(col, order);
        return *this;
    }

    SelectHandle& offset(size_t n) { stmt.offset(n); return *this; }
    SelectHandle& limit(size_t n) { stmt.limit(n); return *this; }

private:
    SelectStmt stmt;
}; // class SelectHandle

class InsertHandle : public Handle {
public:
    InsertHandle(const std::shared_ptr<DBEngine>& engine) : Handle(engine) {}
    ~InsertHandle() {}

    std::shared_ptr<Result> exec() override { return engine_->execute(stmt); }

    InsertHandle& table(const TableStmt& tbl) { stmt.table(tbl); return *this; }

    InsertHandle& column(const ColumnStmt& col) { stmt.column(col); return *this; }
    InsertHandle& column(const std::initializer_list<ColumnStmt>& cols) {
        stmt.column(cols);
        return *this;
    }

    InsertHandle& value(const ValueStmt& val) { stmt.value(val); return *this; }
    InsertHandle& value(const std::initializer_list<ValueStmt>& vals) {
        stmt.value(vals);
        return *this;
    }

    InsertHandle& select(const SelectStmt& slt) { stmt.select(slt); return *this; }

private:
    InsertStmt stmt;
}; // class InsertHandle


class UpdateHandle : public Handle {
public:
    UpdateHandle(const std::shared_ptr<DBEngine>& engine) : Handle(engine) {}
    ~UpdateHandle() {}

    std::shared_ptr<Result> exec() override { return engine_->execute(stmt); }

    UpdateHandle& table(const TableStmt& tbl) { stmt.table(tbl); return *this; }
    UpdateHandle& table(const std::initializer_list<TableStmt>& tbls) {
        stmt.table(tbls);
        return *this;
    }

    UpdateHandle& assign(const AssignStmt& ass) { stmt.assign(ass); return *this; }
    UpdateHandle& assign(const std::initializer_list<AssignStmt>& stmts) {
        stmt.assign(stmts);
        return *this;
    }

    UpdateHandle& where(const CondStmt& cond) { stmt.where(cond); return *this; }
    UpdateHandle& where(const std::initializer_list<CondStmt>& conds) {
        stmt.where(conds);
        return *this;
    }

    UpdateHandle& order_by(const ColumnStmt& col, int order = Order::ASC) {
        stmt.order_by(col, order);
        return *this;
    }

    UpdateHandle& limit(size_t n) { stmt.limit(n); return *this; }

private:
    UpdateStmt stmt;
}; // class UpdateHandle


class DeleteHandle : public Handle {
public:
    DeleteHandle(const std::shared_ptr<DBEngine>& engine) : Handle(engine) {}
    ~DeleteHandle() {}

    std::shared_ptr<Result> exec() override { return engine_->execute(stmt); }

    DeleteHandle& table(const TableStmt& tbl) { stmt.table(tbl); return *this; }

    DeleteHandle& where(const CondStmt& cond) { stmt.where(cond); return *this; }
    DeleteHandle& where(const std::initializer_list<CondStmt>& conds) {
        stmt.where(conds);
        return *this;
    }

    DeleteHandle& order_by(const ColumnStmt& col, int order = Order::ASC) {
        stmt.order_by(col, order);
        return *this;
    }

    DeleteHandle& limit(size_t n) { stmt.limit(n); return *this; }

private:
    DeleteStmt stmt;
}; // class DeleteHandle

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_HANDLE