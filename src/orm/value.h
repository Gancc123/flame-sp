#ifndef FLAME_ORM_VALUE_H
#define FLAME_ORM_VALUE_H

#include "stmt.h"

namespace flame {
namespace orm {

class IntVal : public Value {
public:
    IntVal() : Value(EnumValueTypes::INT), val_(0) {}
    IntVal(int64_t val) : Value(EnumValueTypes::INT), val_(val) {}
    ~IntVal() {}

    IntVal(const IntVal&) = default;
    IntVal(IntVal&&) = default;
    IntVal& operator=(const IntVal&) = default;
    IntVal& operator=(IntVal&&) = default;

    IntVal& operator=(int64_t val) { 
        val_ = val; 
        return *this;
    }
    operator int64_t() const { return val_; }

    virtual std::string to_str() const override { 
        return convert2string(val_);
    }

private:
    int64_t val_;
}; // class IntVal

class UIntVal : public Value {
public:
    UIntVal() : Value(EnumValueTypes::INT), val_(0) {}
    UIntVal(uint64_t val) : Value(EnumValueTypes::UINT), val_(val) {}
    ~UIntVal() {}

    UIntVal(const UIntVal&) = default;
    UIntVal(UIntVal&&) = default;
    UIntVal& operator=(const UIntVal&) = default;
    UIntVal& operator=(UIntVal&&) = default;

    UIntVal& operator=(uint64_t val) { 
        val_ = val; 
        return *this;    
    }
    operator uint64_t() const { return val_; }

    virtual std::string to_str() const override {
        return convert2string(val_);
    }

private:
    uint64_t val_;
}; // class UIntVal

class DoubleVal : public Value {
public:
    DoubleVal() : Value(EnumValueTypes::DOUBLE), val_(0.0) {}
    DoubleVal(double val) : Value(EnumValueTypes::DOUBLE), val_(val) {}
    ~DoubleVal() {}

    DoubleVal(const DoubleVal&) = default;
    DoubleVal(DoubleVal&&) = default;
    DoubleVal& operator=(const DoubleVal&) = default;
    DoubleVal& operator=(DoubleVal&&) = default;

    DoubleVal& operator=(double val) { 
        val_ = val;
        return *this;
    }
    operator double() const { return val_; }

    virtual std::string to_str() const override {
        return convert2string(val_);
    }

private:
    double val_;
}; // class DoubleVal

class StringVal : public Value {
public:
    StringVal() : Value(EnumValueTypes::STRING), str_() {}
    StringVal(const std::string& str) : Value(EnumValueTypes::STRING), str_(str) {}
    StringVal(const char* str) : Value(EnumValueTypes::STRING), str_(str) {}
    ~StringVal() {}

    StringVal(const StringVal&) = default;
    StringVal(StringVal&&) = default;
    StringVal& operator=(const StringVal&) = default;
    StringVal& operator=(StringVal&&) = default;

    StringVal& operator=(const std::string& str) { 
        str_ = str; 
        return *this;
    }
    operator std::string() const { return str_; }

    virtual std::string to_str() const override {
        return string_concat({"\'", string_encode_ez(str_, '\''), "\'"});
    }

    bool empty() const { return str_.empty(); }

private:
    std::string str_;
}; // class StringVal

class NullVal : public Value {
public:
    NullVal() : Value(EnumValueTypes::STRING) {}
    ~NullVal() {}

    NullVal(const NullVal&) = default;
    NullVal(NullVal&&) = default;
    NullVal& operator=(const NullVal&) = default;
    NullVal& operator=(NullVal&&) = default;

    virtual std::string to_str() const override { return "NULL"; }
}; // class StringVal

} // namespace orm
} // namespace flame

#endif // FLAME_ORM_VALUE_H