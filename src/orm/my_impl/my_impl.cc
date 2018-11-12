#include "my_impl.h"

#include "util/clog.h"
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cstdio>


#define my_error_log(e) mysql_error_(e, __FILE__, __LINE__)

namespace flame {
namespace orm {

static inline void mysql_error_(const sql::SQLException& e, const std::string& file, int line) {
    std::ostringstream oss;
    oss << "MySQL Error: File(" << file << ") Line(" << line 
        << ") Code(" << e.getErrorCode() << ") State(" << e.getSQLState()
        << ") " << e.what();
    clog(oss.str());
}

static inline void my_impl_log(const std::string& cls, const std::string& msg) {
    clog(string_concat({"MySQL [", cls, "] ", msg}));
}

std::shared_ptr<Result> MysqlStub::execute(const std::string& stmt) {
    my_impl_log("execute", stmt);
    assert(conn_ != nullptr);
    std::unique_ptr<sql::Statement> handle(conn_->createStatement());
    int code = RetCode::OK;
    if (!handle->execute(stmt)) {
        code = RetCode::FAILD;
    }
    return std::shared_ptr<Result>(new MysqlResult(code, 0, nullptr));
}

std::shared_ptr<Result> MysqlStub::execute_query(const std::string& stmt) {
    my_impl_log("execute_query", stmt);
    std::unique_ptr<sql::Statement> handle(conn_->createStatement());
    std::shared_ptr<sql::ResultSet> res(handle->executeQuery(stmt));
    if (res.get())
        return std::shared_ptr<Result>(new MysqlResult(
            RetCode::OK, 
            res->rowsCount(), 
            std::shared_ptr<MysqlDataSet>(new MysqlDataSet(res))
        ));
    return std::shared_ptr<Result>(new MysqlResult(RetCode::FAILD, 0, nullptr)); 
}

std::shared_ptr<Result> MysqlStub::execute_update(const std::string& stmt) {
    my_impl_log("execute_update", stmt);
    assert(conn_ != nullptr);
    std::unique_ptr<sql::Statement> handle(conn_->createStatement());
    size_t len = handle->executeUpdate(stmt);
    return std::shared_ptr<Result>(new MysqlResult(RetCode::OK, len, nullptr));
}

std::shared_ptr<Stub> MysqlDriver::create_stub() const {
    try {
        sql::Connection* conn = driver_->connect(path_, user_, passwd_);
        my_impl_log("create_stub", string_concat({path_, "?user=", user_, "&passwd=", passwd_}));
        if (!conn)
            return nullptr;
        conn->setSchema(schema_);
        return std::shared_ptr<Stub>(new MysqlStub(conn));
    } catch (sql::SQLException& e) {
        my_error_log(e);
    }
    return nullptr;
}


} // namespace orm
} // namespace flame