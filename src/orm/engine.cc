#include "engine.h"
#include "driver.h"
#include "my_impl/my_impl.h"

#include "util/clog.h"

#include <cassert>
#include <regex>

namespace flame {
namespace orm {

bool DBUrlParser::match(const std::string& text) {
    if (std::regex_match(text, result_, pattern_)) {
        matched_ = true;
    } else {
        matched_ = false;
    }
    return matched_;
}

std::shared_ptr<DBEngine> DBEngine::create_engine(const std::string& url, size_t stub_num) {
    DBUrlParser parser(url);
    if (!parser.matched()) {
        clog("Engine ERR: DB url format error!");
        return nullptr;
    }


    Driver* driver = nullptr;
    std::string driver_type = parser.driver();
    if (driver_type == "mysql") {
        std::string addr = "tcp://";
        addr += parser.host();
        std::string port = parser.port();
        if (!port.empty()) {
            addr += ":";
            addr += port;
        }
        driver = new MysqlDriver(addr, parser.db(), parser.user(), parser.passwd());
    } else {
        clog("Engine ERR: DB Driver is not found!");
        return nullptr;
    }

    assert(stub_num > 0);
    std::shared_ptr<DBEngine> engine(new DBEngine(driver));
    
    size_t created = engine->create_stub(stub_num);
    if (created < stub_num) {
        clog("Engine ERR: create stub faild, check url and your db server!");
        return nullptr;
    }
    
    return engine;
}

std::shared_ptr<Result> DBEngine::execute(const std::string& stmt) {
    StubOwner owner(stub_pool_);
    return owner.get_stub()->execute(stmt);
}

std::shared_ptr<Result> DBEngine::execute_query(const std::string& stmt) {
    StubOwner owner(stub_pool_);
    return owner.get_stub()->execute_query(stmt);
}

std::shared_ptr<Result> DBEngine::execute_update(const std::string& stmt) {
    StubOwner owner(stub_pool_);
    return owner.get_stub()->execute_update(stmt);
}

std::shared_ptr<Result> DBEngine::execute(const SelectStmt& stmt) {
    StubOwner owner(stub_pool_);
    return owner.get_stub()->execute(stmt);
}

std::shared_ptr<Result> DBEngine::execute(const InsertStmt& stmt) {
    StubOwner owner(stub_pool_);
    return owner.get_stub()->execute(stmt);
}

std::shared_ptr<Result> DBEngine::execute(const UpdateStmt& stmt) {
    StubOwner owner(stub_pool_);
    return owner.get_stub()->execute(stmt);
}

std::shared_ptr<Result> DBEngine::execute(const DeleteStmt& stmt) {
    StubOwner owner(stub_pool_);
    return owner.get_stub()->execute(stmt);
}

void DBEngine::StubPool::push(const std::shared_ptr<Stub>& stub) {
    MutexLocker locker(stub_que_mutex_);
    stub_que_.push(stub);
    stub_que_cond_.signal();
}

bool DBEngine::StubPool::pop(std::shared_ptr<Stub>& stub) {
    MutexLocker locker(stub_que_mutex_);
    if (stub_que_.size() == 0)
        return false;
    stub = stub_que_.front();
    stub_que_.pop();
    return true;
}

std::shared_ptr<Stub> DBEngine::StubPool::pop() {
    MutexLocker locker(stub_que_mutex_);

    while (stub_que_.size() == 0) {
        stub_que_cond_.wait();
    }

    std::shared_ptr<Stub> stub = stub_que_.front();
    stub_que_.pop();
    return stub;
}

size_t DBEngine::create_stub(size_t count) {
    for (size_t i = 0; i < count; i++) {
        std::shared_ptr<Stub> stub = driver_->create_stub();
        if (!stub) {
            clog("create stub faild.");
            break;
        }
        stub_pool_.push(stub);
    }
    return stub_pool_.count();
}

} // namespace orm
} // namespace flame