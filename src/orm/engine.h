#ifndef FLAME_ORM_ENGINE_H
#define FLAME_ORM_ENGINE_H

#include "driver.h"
#include "common/context.h"
#include "common/thread/mutex.h"
#include "common/thread/cond.h"

#include <string>
#include <memory>
#include <queue>
#include <regex>

#define DBE_DEF_PASSWD ""
#define DBE_DEF_PORT   3306
#define DBE_URL_PATTERN "^(\\w+)://((\\w+)(:\\w+)?)?((@[^@:]+)(:\\d+)?)?(/.+)$"
#define DBE_DEF_CONNNUM 8

namespace flame {
namespace orm {

class DBUrlParser final {
public:
    DBUrlParser() : matched_(false), pattern_(DBE_URL_PATTERN) {}
    explicit DBUrlParser(const std::string& text) 
    : matched_(false), pattern_(DBE_URL_PATTERN) {
        match(text);
    }

    bool matched() { return matched_; }
    bool match(const std::string& text);

    std::string text() const { return result_[0]; }
    std::string driver() const { return result_[1]; }
    std::string user() const { return result_[3]; }
    std::string passwd() const { return substr_(result_[4]); }
    std::string host() const { return substr_(result_[6]); }
    std::string port() const { return substr_(result_[7]); }
    std::string db() const { return substr_(result_[8]); }
    std::string path() const { return substr_(result_[8]); }

private:
    std::string substr_(const std::string& str) const {
        if (!str.empty())
            return str.substr(1);
        return "";
    }

    bool matched_;
    std::regex pattern_;
    std::smatch result_;
};

class DBEngine final {
public:
    /**
     * Create An DBengine
     * @url format:
     *      <driver>://<user>[:passwd]@<host>[:port]/<database>
     * @driver, @user, @host and @database is requisite
     * default @passwd is ""
     * default @port is 3306
     * @dirver could be seted as "mysql",
     * eg:
     *      mysql://flame:123456@localhost/flame_mgr_db
     */
    static std::shared_ptr<DBEngine> create_engine(FlameContext* fct, const std::string& url, size_t conn_num = DBE_DEF_CONNNUM);

    ~DBEngine() {}

    std::shared_ptr<Result> execute(const std::string& stmt);
    std::shared_ptr<Result> execute_query(const std::string& stmt);
    std::shared_ptr<Result> execute_update(const std::string& stmt);
    std::shared_ptr<Result> execute(const SelectStmt& stmt);
    std::shared_ptr<Result> execute(const InsertStmt& stmt);
    std::shared_ptr<Result> execute(const UpdateStmt& stmt);
    std::shared_ptr<Result> execute(const DeleteStmt& stmt);
    std::shared_ptr<Result> execute(const MultiInsertStmt& stmt);

    DBEngine(const DBEngine&) = delete;
    DBEngine(DBEngine&&) = delete;
    DBEngine& operator = (const DBEngine&) = delete;
    DBEngine& operator = (DBEngine&&) = delete;

private:
    class StubPool {
    public:
        StubPool() { stub_que_cond_.set_mutex(stub_que_mutex_); }
        ~StubPool() {}

        size_t count() const { return stub_que_.size(); }

        void push(const std::shared_ptr<Stub>& stub);

        // non-blocking
        bool pop(std::shared_ptr<Stub>& stub);
        
        // blocking
        std::shared_ptr<Stub> pop();

    private:
        std::queue<std::shared_ptr<Stub>> stub_que_;
        Mutex stub_que_mutex_;
        Cond stub_que_cond_;
    }; // class StubPool

    class StubOwner {
    public:
        StubOwner(StubPool& pool) : pool_(pool) { stub_ = pool_.pop(); }

        ~StubOwner() { pool_.push(stub_); }

        std::shared_ptr<Stub> get_stub() const { return stub_; }
    
    private:
        StubPool& pool_;
        std::shared_ptr<Stub> stub_;
    };

    DBEngine(FlameContext* fct, Driver* driver) : fct_(fct), driver_(driver) {}

    size_t stub_count() const { return stub_pool_.count(); }
    
    // Return real number of stub that created successfully.
    size_t create_stub(size_t count = 1);

    FlameContext* fct_;
    Driver* driver_;
    StubPool stub_pool_;
}; // class DBEngine

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_ENGINE_H