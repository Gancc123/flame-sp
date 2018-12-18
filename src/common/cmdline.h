#ifndef FLAME_COMMON_CMDLINE_H
#define FLAME_COMMON_CMDLINE_H

#include "util/str_util.h"
#include "util/clog.h"

#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace flame {
namespace cli {

/**
 * @sn: short name, not support short name iff sn == 0
 *  -sn
 * @ln: long name, not support long name iff ln.empty()
 *  --ln
 * @dv: default
 * @des: description
 */
class Cmdline;

enum CmdType {
    SWITCH = 0,
    ARGUMENT = 1,
    SERIAL = 2,
    ACTION = 3
};

enum CmdRetCode {
    SUCCESS = 0,
    FORMAT_ERROR = -1,
    DEF_ERROR = -2
};

class CmdBase {
public:
    virtual ~CmdBase() {}

    virtual std::string get_def() const = 0;

    bool done() const { return done_; }
    void set_done() { done_ = true; }
    int cmd_type() const { return t_; }
    char short_name() const { return sn_; }
    std::string long_name() const { return ln_; }
    std::string des() const { return des_; }

protected:
    CmdBase(Cmdline* cmd, int t, char sn, const std::string& ln, const std::string& des)
    : cmd_(cmd), sn_(sn), ln_(ln), des_(des), t_(t), done_(false) {
        assert_msg(!ln_.empty(), "Cmdline: long name can't be empty!");
    }

    virtual void register_self();

    Cmdline* cmd_;
    char sn_;
    std::string ln_;
    std::string des_;
    int t_;
    bool done_;
};

class Switch final : public CmdBase {
public:
    Switch(Cmdline* cmd, const std::string& ln, const std::string& des)
    : CmdBase(cmd, CmdType::SWITCH, '\0', ln, des), val_(false) {
        register_self();
    }

    Switch(Cmdline* cmd, char sn, const std::string& ln, const std::string& des)
    : CmdBase(cmd, CmdType::SWITCH, sn, ln, des), val_(false) {
        register_self();
    }

    ~Switch() {}

    virtual std::string get_def() const override { return val_ ? "true" : "false"; }

    bool default_value() const { return val_; }

    bool get() const { return val_; }
    void set(bool val) { val_ = val; }

    operator bool () const { return val_; }

private:
    
    bool val_;
}; // class Switch

template<typename T>
bool def_trans_func(T& val, const std::string& str) {
    return false;
}

template<>
bool def_trans_func<std::string>(std::string& val, const std::string& str);

template<>
bool def_trans_func<int>(int& val, const std::string& str);

template<>
bool def_trans_func<long>(long& val, const std::string& str);

template<>
bool def_trans_func<unsigned long>(unsigned long& val, const std::string& str);

template<>
bool def_trans_func<long long>(long long& val, const std::string& str);

template<>
bool def_trans_func<unsigned long long>(unsigned long long& val, const std::string& str);

template<>
bool def_trans_func<float>(float& val, const std::string& str);

template<>
bool def_trans_func<double>(double& val, const std::string& str);

template<>
bool def_trans_func<long double>(long double& val, const std::string& str);

class ArgumentBase : public CmdBase {
public:
    virtual ~ArgumentBase() {}

    bool is_must() const { return must_; }
    virtual bool set_with_str(const std::string& str) = 0;

protected:
    ArgumentBase(Cmdline* cmd, bool must, char sn, const std::string& ln, const std::string& des)
    : CmdBase(cmd, CmdType::ARGUMENT, sn, ln, des), must_(must) {
    }

    bool must_;
}; // class ArgumentBase

template<typename T>
class Argument final : public ArgumentBase {
public:
    typedef bool (*trans_func_t)(T&, const std::string&);

    Argument(Cmdline* cmd, const std::string& ln, const std::string& des, 
    trans_func_t func = def_trans_func<T>)
    : ArgumentBase(cmd, true, '\0', ln, des), val_(), def_(), func_(func) {
        register_self();
    }

    Argument(Cmdline* cmd, const std::string& ln, const std::string& des, T dv,
    trans_func_t func = def_trans_func<T>)
    : ArgumentBase(cmd, false, '\0', ln, des), val_(dv), def_(dv), func_(func) {
        register_self();
    }

    Argument(Cmdline* cmd, char sn, const std::string& ln, const std::string& des, 
    trans_func_t func = def_trans_func<T>)
    : ArgumentBase(cmd, true, sn, ln, des), val_(), def_(), func_(func) {
        register_self();
    }

    Argument(Cmdline* cmd, char sn, const std::string& ln, const std::string& des, T dv,
    trans_func_t func = def_trans_func<T>)
    : ArgumentBase(cmd, false, sn, ln, des), val_(dv), def_(dv), func_(func) {
        register_self();
    }

    virtual std::string get_def() const override { return convert2string(def_); }

    T def_val() const { return def_; }
    T get() const { return val_; }
    void set(T val) { val_ = val; }
    virtual bool set_with_str(const std::string& str) override {
        if (func_ != nullptr)
            return func_(val_, str);
        return false;
    }

    operator T () const { return val_; }
    
private:
    T val_;
    T def_;
    trans_func_t func_;
}; // class Argument

class SerialBase : public CmdBase {
public:
    virtual ~SerialBase() {}

    int get_idx() const { return idx_; }
    bool is_must() const { return must_; }
    virtual bool set_with_str(const std::string& str) = 0;

protected:
    SerialBase(Cmdline* cmd, bool must, int index, const std::string& ln, const std::string& des)
    : CmdBase(cmd, CmdType::SERIAL, '\0', ln, des), idx_(index), must_(must) {}

    int idx_;
    bool must_;
}; // class SerialBase

template<typename T>
class Serial final : public SerialBase {
public:
    typedef bool (*trans_func_t)(T& dst, const std::string& src);

    Serial(Cmdline* cmd, int index, const std::string& ln, const std::string& des,
    trans_func_t func = def_trans_func<T>)
    : SerialBase(cmd, true, index, ln, des), val_(), def_(), func_(func) {
        register_self();
    }

    Serial(Cmdline* cmd, int index, const std::string& ln, const std::string& des,  T dv,
    trans_func_t func = def_trans_func<T>)
    : SerialBase(cmd, false, index, ln, des), val_(dv), def_(dv), func_(func) {
        register_self();
    }

    virtual std::string get_def() const override { return convert2string(def_); }

    T def_val() const { return def_; }
    T get() const { return val_; }
    void set(T val) { val_ = val; }
    bool set_with_str(const std::string& str) {
        if (func_ != nullptr)
            return func_(val_, str);
        return false;
    }

    operator T () const { return val_; }

private:
    T val_;
    T def_;
    trans_func_t func_;
}; // class Serial

typedef int (*act_funct_t)(Cmdline* cmd);

class Action : public CmdBase {
public:
    Action(Cmdline* cmd, const std::string& ln, const std::string& des,
    act_funct_t func = nullptr)
    : CmdBase(cmd, CmdType::ACTION, '\0', ln, des), func_(func) {
        register_self();
    }

    Action(Cmdline* cmd, char sn, const std::string& ln, const std::string& des,
    act_funct_t func = nullptr)
    : CmdBase(cmd, CmdType::ACTION, sn, ln, des), func_(func) {
        register_self();
    }

    virtual ~Action() {}

    virtual std::string get_def() const { return ln_; }
    virtual int run(Cmdline* cmd);

protected:
    act_funct_t func_;
}; // class

class Cmdline {
public:
    virtual ~Cmdline() {}

    virtual int def_run();

    Cmdline* parent() const { return parent_; }
    Cmdline* active_module() const { return active_; }
    std::string active_name() const { return active_module()->name(); }

    std::string name() const { return name_; }
    std::string des() const { return des_; }
    void register_self(Cmdline* parent);
    void register_type(CmdBase* arg);
    void register_submodule(Cmdline* sub, const std::string& name);
    
    bool has_tail() const { return tail_; }
    int tail_size() const { return tail_vec_.size(); }
    std::string tail_list(int idx) const { return tail_vec_[idx]; }
    void print_help();
    void print_error();
    void print_internal_error();

    int parser(int argc, char** argv);
    int run(int argc, char** argv); 

protected:
    Cmdline(const std::string& name, const std::string& des, bool tail = false) 
    : max_idx_(0), name_(name), des_(des), active_(this), parent_(nullptr), tail_(tail), act_(nullptr) {}
    
    Cmdline(Cmdline* parent, const std::string& name, const std::string& des, bool tail = false) 
    : max_idx_(0), name_(name), des_(des), active_(this), parent_(parent), tail_(tail), act_(nullptr) {
        assert_msg(parent != nullptr, "Cmdline: Cmdline's parent can't be set with nullptr explicitly!");
        parent->register_submodule(this, name);
    }

    std::string name_;
    std::string des_;
    std::map<char, CmdBase*> sn_map_;
    std::map<std::string, CmdBase*> ln_map_;
    std::map<int, SerialBase*> idx_map_;
    std::map<std::string, Cmdline*> sub_map_;
    std::vector<std::string> tail_vec_;
    Action* act_;
    int max_idx_;
    Cmdline* active_;
    Cmdline* parent_;
    bool tail_;

    std::ostringstream err_msg_;

private:
    bool check_def__();
    Cmdline* check_submodule__(const std::string& str);
    Cmdline* top_module__();
    void change_active__();
    int do_parser__(int argc, char** argv);
    
}; // class Cmline

class HelpAction final : public Action {
public:
    HelpAction(Cmdline* cmd) : Action(cmd, 'h', "help", "print this help doc") {}
    ~HelpAction() {}

    virtual int run(Cmdline* cmd) override {
        cmd->lprint_help();
        return 0;
    }
}; // class 

} // namespace cli
} // namespace flame

#endif // FLAME_COMMON_CMDLINE_H