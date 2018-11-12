#ifndef FLAME_ORM_DRIVER_H
#define FLAME_ORM_DRIVER_H

#include <cassert>
#include <string>
#include <iterator>
#include <memory>

#include "cols.h"
#include "stmt.h"

namespace flame {
namespace orm {

class DataSet;
class Result;

/**
 * Record
 * not thread safe
 */
class Record {
public:
    virtual ~Record() {}

    // Return the number of Columns
    virtual size_t cols() const = 0;

    virtual int64_t get_int(size_t idx) const = 0;
    virtual uint64_t get_uint(size_t idx) const = 0;
    virtual double get_double(size_t idx) const = 0;
    virtual std::string get_str(size_t idx) const = 0;

    virtual int64_t get_int(const std::string& label) const = 0;
    virtual uint64_t get_uint(const std::string& label) const = 0;
    virtual double get_double(const std::string& label) const = 0;
    virtual std::string get_str(const std::string& label) const = 0;

    virtual int64_t get(const IntColumn& col) const { 
        return get_int(col.col_name()); 
    }
    virtual uint64_t get(const UIntColumn& col) const { 
        return get_uint(col.col_name());
    }
    virtual double get(const DecimalColumn& col) const {
        return get_double(col.col_name());
    }
    virtual std::string get(const StringColumn& col) const {
        return get_str(col.col_name());
    }
// protected:
//     Record(const std::shared_ptr<DataSet>& ds) : ds_(ds) {}

//     std::shared_ptr<DataSet> ds_;
}; // class Record

/**
 * DataSet
 * not thread safe
 */
class DataSet {
public:
    virtual ~DataSet() {}

    // Return the number of Records
    virtual size_t rows() const = 0;

    // Return the number of Columns
    virtual size_t cols() const = 0;

    // Return the 
    virtual std::string col_name(size_t idx) const = 0;

    // idx in range of [0, size)
    virtual std::shared_ptr<Record> get_row(size_t idx) const =  0;

    class const_iterator : public std::iterator<std::random_access_iterator_tag, Record> {
        DataSet* ds_;
        ssize_t idx_;
    public:
        const_iterator(DataSet* ds, ssize_t idx) : ds_(ds), idx_(idx) {}
        
        const_iterator(const const_iterator&) = default;
        const_iterator(const_iterator&&) = default;
        const_iterator& operator = (const const_iterator&) = default;
        const_iterator& operator = (const_iterator&&) = default;

        bool operator == (const const_iterator& rhs) const { 
            return ds_ == rhs.ds_ && idx_ == rhs.idx_;
        }
        bool operator != (const const_iterator& rhs) const { 
            return !(*this == rhs); 
        }

        std::shared_ptr<Record> operator * () const { return ds_->get_row(idx_); }
        std::shared_ptr<Record> operator -> () const { return ds_->get_row(idx_); }

        const_iterator& operator ++ () { 
            ++idx_; 
            return *this; 
        }
        const_iterator operator ++ (int) { 
            const_iterator tmp(*this); 
            operator++(); 
            return tmp; 
        }

        const_iterator& operator -- () { 
            --idx_; 
            return *this; 
        }
        const_iterator operator -- (int) { 
            const_iterator tmp(*this); 
            operator--(); 
            return tmp; 
        }

        const_iterator operator + (ssize_t d) const {
            const_iterator tmp(*this);
            tmp.idx_ += d;
            return tmp;
        }
        const_iterator operator - (ssize_t d) const {
            const_iterator tmp(*this);
            tmp.idx_ -= d;
            return tmp;
        }
        ssize_t operator - (const const_iterator& rhs) const { return idx_ - rhs.idx_; }
        const_iterator& operator += (ssize_t d) { idx_ += d; return *this; }
        const_iterator& operator -= (ssize_t d) { idx_ -= d; return *this; }

        bool operator < (const const_iterator& rhs) const { 
            return ds_ == rhs.ds_ && idx_ < rhs.idx_; 
        }
        bool operator <= (const const_iterator& rhs) const { 
            return ds_ == rhs.ds_ && idx_ <= rhs.idx_; 
        }
        bool operator > (const const_iterator& rhs) const { 
            return ds_ == rhs.ds_ && idx_ > rhs.idx_; 
        }
        bool operator >= (const const_iterator& rhs) const { 
            return ds_ == rhs.ds_ && idx_ >= rhs.idx_; 
        }

        std::shared_ptr<Record> operator[] (size_t d) const { return ds_->get_row(idx_ + d); }
    }; // class iterator

    const_iterator cbegin() { return const_iterator(this, 0); }
    const_iterator cend() { return const_iterator(this, rows()); }

protected:
    DataSet() {}
}; // class DataSet

enum RetCode {
    OK = 0,
    FAILD = 1
};

/**
 * Result
 */
class Result {
public:
    virtual ~Result() {}

    int code() const { return code_; }
    size_t len() const { return len_; }
    virtual bool has_ds() const = 0;
    virtual std::shared_ptr<DataSet> data_set() const = 0;

    bool OK() { return code() == 0; }

    Result(const Result&) = delete;
    Result(Result&&) = delete;
    Result& operator=(const Result&) = delete;
    Result& operator=(Result&&) = delete;

protected:
    Result(int code, size_t len) 
        : code_(code), len_(len) {}

    int code_;      // retcode
    
    // len_ equal to rows of DataSet iff stmt is select, 
    // otherwise len_ equal to the number of affected rows.
    size_t len_;
}; // class Result

/**
 * Stub
 */
class Stub {
public:
    virtual ~Stub() {}

    virtual std::shared_ptr<Result> execute(const std::string& stmt) = 0;
    virtual std::shared_ptr<Result> execute_query(const std::string& stmt) = 0;
    virtual std::shared_ptr<Result> execute_update(const std::string& stmt) = 0;

    std::shared_ptr<Result> execute(const SelectStmt& stmt) { return execute_query(stmt.to_str()); }
    std::shared_ptr<Result> execute(const InsertStmt& stmt) { return execute_update(stmt.to_str()); }
    std::shared_ptr<Result> execute(const UpdateStmt& stmt) { return execute_update(stmt.to_str()); }
    std::shared_ptr<Result> execute(const DeleteStmt& stmt) { return execute_update(stmt.to_str()); }

protected:
    Stub() {}
}; // class Stub

/**
 * Driver
 * For adapting other SQL Engine, eg. MySQL, SQLite
 */
class Driver {
public:
    virtual ~Driver() {}

    std::string name() const { return name_; }
    std::string path() const { return path_; }

    virtual std::shared_ptr<Stub> create_stub() const = 0;

protected:
    Driver(const std::string& name, const std::string& path) : name_(name), path_(path) {}
    
    std::string name_;  // driver name
    std::string path_;  // driver path
}; // class Driver

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_DRIVER_H