#ifndef FLAME_ORM_TABLE_H
#define FLAME_ORM_TABLE_H

#include "stmt.h"
#include "engine.h"
#include "handle.h"

#include <memory>

namespace flame {
namespace orm {

class DBTable : public Table {
public:
    DBTable(const std::shared_ptr<DBEngine>& engine, const std::string& table_name)
    : Table(table_name), engine_(engine) {}
    virtual ~DBTable() {}

    SelectHandle query()  { return SelectHandle(engine_).table(*this); }
    InsertHandle insert() { return InsertHandle(engine_).table(*this); }
    MultiInsertHandle multi_insert() { return MultiInsertHandle(engine_).table(*this); }
    UpdateHandle update() { return UpdateHandle(engine_).table(*this); }
    DeleteHandle remove() { return DeleteHandle(engine_).table(*this); }

    bool auto_create() { 
        try_create(); 
    }
    
    bool recreate() { 
        try_drop(); try_create(); 
    }

private:
    bool try_create() {
        std::shared_ptr<Result> res = engine_->execute(stmt_create());
        return res->code() == RetCode::OK;
    }

    bool try_drop() {
        std::shared_ptr<Result> res = engine_->execute(stmt_drop());
        return res->code() == RetCode::OK;
    }

    std::shared_ptr<DBEngine> engine_;
}; // class DBTable

} // namespace orm 
} // namespace flame

#endif // FALME_ORM_TABLE_H