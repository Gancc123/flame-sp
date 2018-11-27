#ifndef FLAME_ORM_MY_IMPL_MY_IMPL_H
#define FLAME_ORM_MY_IMPL_MY_IMPL_H

#include <string>

#include "common/context.h"
#include "orm/driver.h"
#include "public.h"

namespace flame {
namespace orm {

class MysqlRecord final : public Record {
public:
    virtual ~MysqlRecord() {}

    // Return the number of Columns
    virtual size_t cols() const override { return cols_; }

    virtual int64_t get_int(size_t idx) const override {
        change_row_();
        return data_->getInt64(idx + 1); 
    }
    virtual uint64_t get_uint(size_t idx) const override {
        change_row_();
        return data_->getUInt64(idx + 1);
    }
    virtual double get_double(size_t idx) const override {
        change_row_();
        return data_->getDouble(idx + 1);
    }
    virtual std::string get_str(size_t idx) const override {
        change_row_();
        return data_->getString(idx - 1);
    }

    virtual int64_t get_int(const std::string& label) const override {
        change_row_();
        return data_->getInt64(label);
    }
    virtual uint64_t get_uint(const std::string& label) const override {
        change_row_();
        return data_->getUInt64(label);
    }
    virtual double get_double(const std::string& label) const override {
        change_row_();
        return data_->getDouble(label);
    }
    virtual std::string get_str(const std::string& label) const override {
        change_row_();
        return data_->getString(label);
    }

    friend class MysqlDataSet;
private:
    MysqlRecord(const std::shared_ptr<sql::ResultSet>& data, size_t row, size_t cols) 
        : data_(data), row_(row), cols_(cols) {}

    bool change_row_() const { data_->absolute(row_ + 1); }

    std::shared_ptr<sql::ResultSet> data_;
    size_t row_;
    size_t cols_;
}; // class MysqlRecord

class MysqlDataSet final : public DataSet {
public:
    virtual ~MysqlDataSet() {}

    virtual size_t rows() const override { return data_->rowsCount(); }
    virtual size_t cols() const override { return meta_->getColumnCount(); }
    virtual std::string col_name(size_t idx) const override {
        return meta_->getColumnName(idx + 1);
    }
    virtual std::shared_ptr<Record> get_row(size_t idx) const override {
        return std::shared_ptr<Record>(new MysqlRecord(data_, idx, cols()));
    }
 
    friend class MysqlResult;
    friend class MysqlStub;

private:
    MysqlDataSet(const std::shared_ptr<sql::ResultSet>& data) : data_(data) {
        meta_ = data->getMetaData();
    }

    std::shared_ptr<sql::ResultSet> data_;
    sql::ResultSetMetaData* meta_;
}; // class MysqlDataSet

class MysqlResult final : public Result {
public:
    virtual ~MysqlResult() {}
    
    virtual bool has_ds() const override { return ds_.get() == nullptr; }
    virtual std::shared_ptr<DataSet> data_set() const override { return ds_; }

    MysqlResult(const MysqlResult&) = delete;
    MysqlResult(MysqlResult&&) = delete;
    MysqlResult& operator = (const MysqlResult&) = delete;
    MysqlResult& operator = (MysqlResult&&) = delete;

    friend class MysqlStub;

private:
    MysqlResult(int code, size_t len, const std::shared_ptr<MysqlDataSet>& ds)
        : Result(code, len), ds_(ds) {}

    std::shared_ptr<MysqlDataSet> ds_;
}; // class MysqlResult

class MysqlStub final : public Stub {
public:
    ~MysqlStub() { delete conn_; }

    virtual std::shared_ptr<Result> execute(const std::string& stmt) override;
    virtual std::shared_ptr<Result> execute_query(const std::string& stmt) override;
    virtual std::shared_ptr<Result> execute_update(const std::string& stmt) override;

private:
    MysqlStub(FlameContext* fct, sql::Connection* conn) 
    : Stub(fct), conn_(conn) {}

    friend class MysqlDriver;

    sql::Connection* conn_;
}; // class MysqlStub

class MysqlDriver final : public Driver {
public:
    MysqlDriver(FlameContext* fct, const std::string& addr, const std::string& schema, const std::string& user, const std::string& passwd)
        : Driver(fct, "MySQL", addr), schema_(schema), user_(user), passwd_(passwd)
    {
        driver_ = sql::mysql::get_driver_instance();
    }

    std::string addr() const { return path_; }
    std::string schema() const { return schema_; }
    std::string user() const { return user_; }
    std::string passwd() const { return passwd_; }

    virtual std::shared_ptr<Stub> create_stub() const override;

private:
    sql::Driver* driver_;
    std::string schema_;
    std::string user_;
    std::string passwd_;
}; // class MysqlDriver

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_MYSQL_DRIVER_MYSQL_DRIVER_H