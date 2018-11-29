#include "my_impl.h"

#include "util/clog.h"
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cstdio>

#include "log_mysql.h"


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
    fct_->log()->info("[execute] %s", stmt.c_str());
    try {
        std::unique_ptr<sql::Statement> handle(conn_->createStatement());
        int code = RetCode::OK;
        if (!handle->execute(stmt)) {
            code = RetCode::FAILD;
        }
        return std::shared_ptr<Result>(new MysqlResult(code, 0, nullptr));
    } catch (sql::SQLException& e) {
        fct_->log()->error("[execute] Code(%d) Status(%d) %s", e.getErrorCode(), e.getSQLState(), e.what());
    }
    return std::shared_ptr<Result>(new MysqlResult(RetCode::FAILD, 0, nullptr)); 
}

std::shared_ptr<Result> MysqlStub::execute_query(const std::string& stmt) {
    fct_->log()->info("[execute_query] %s", stmt.c_str());
    try {
        std::unique_ptr<sql::Statement> handle(conn_->createStatement());
        std::shared_ptr<sql::ResultSet> res(handle->executeQuery(stmt));
        if (res.get())
            return std::shared_ptr<Result>(new MysqlResult(
                RetCode::OK, 
                res->rowsCount(), 
                std::shared_ptr<MysqlDataSet>(new MysqlDataSet(res))
            ));
    } catch (sql::SQLException& e) {
        fct_->log()->error("[execute_query] Code(%d) Status(%d) %s", e.getErrorCode(), e.getSQLState(), e.what());
    }
    return std::shared_ptr<Result>(new MysqlResult(RetCode::FAILD, 0, nullptr)); 
}

std::shared_ptr<Result> MysqlStub::execute_update(const std::string& stmt) {
    fct_->log()->info("[execute_update] %s", stmt.c_str());
    try {
        std::unique_ptr<sql::Statement> handle(conn_->createStatement());
        size_t len = handle->executeUpdate(stmt);
        return std::shared_ptr<Result>(new MysqlResult(RetCode::OK, len, nullptr));
    } catch (sql::SQLException& e) {
        fct_->log()->error("[execute_update] Code(%d) Status(%d) %s", e.getErrorCode(), e.getSQLState(), e.what());
    }
    return std::shared_ptr<Result>(new MysqlResult(RetCode::FAILD, 0, nullptr));
}

std::shared_ptr<Stub> MysqlDriver::create_stub() const {
    try {
        sql::Connection* conn = driver_->connect(path_, user_, passwd_);
        fct_->log()->info("[create_stub] path=%s user=%s passwd=%s", path_.c_str(), user_.c_str(), passwd_.c_str());
        if (!conn)
            return nullptr;
        conn->setSchema(schema_);
        return std::shared_ptr<Stub>(new MysqlStub(fct_, conn));
    } catch (sql::SQLException& e) {
        fct_->log()->error("[create_stub] Code(%d) Status(%d) %s", e.getErrorCode(), e.getSQLState(), e.what());
    }
    return nullptr;
}


} // namespace orm
} // namespace flame