#include "cmdline.h"

#include <cstring>
#include <cstdio>
#include <cassert>
#include <list>

using namespace std;

namespace flame {
namespace cli {

void CmdBase::register_self() {
    cmd_->register_type(this);
}

int Action::run(Cmdline* cmd) {
    if (func_ != nullptr)
        return func_(cmd);
    return CmdRetCode::SUCCESS;
}

int Cmdline::def_run() {
    print_help();
    return 0;
}

void Cmdline::register_self(Cmdline* parent) {
    assert_msg(parent != nullptr, "Cmdline Internal Error!");
    parent->register_submodule(this, name_);
}

void Cmdline::register_type(CmdBase* arg) {
    assert_msg(arg != nullptr, "Cmdline Internal Error!");
    
    if (arg->cmd_type() == CmdType::SERIAL) {
        SerialBase* ser = dynamic_cast<SerialBase*>(arg);
        assert_msg(ser->get_idx() > 0, "Cmline: Serial's index must bigger than 0!");
        assert_msg(idx_map_.find(ser->get_idx()) == idx_map_.end(), "Cmdline: Redefined Serial's index!");
        assert_msg(ln_map_.find(ser->long_name()) == ln_map_.end(), "Cmdline: Redefined long name!");
        idx_map_[ser->get_idx()] = ser;
        if (ser->get_idx() > max_idx_)
            max_idx_ = ser->get_idx();
        ln_map_[ser->long_name()] = arg;
        return;
    }

    if (arg->short_name() != '\0') {
        assert_msg(sn_map_.find(arg->short_name()) == sn_map_.end(), "Cmdline: Redefined short name!");
        sn_map_[arg->short_name()] = arg;
    }
    if (!arg->long_name().empty()) {
        assert_msg(ln_map_.find(arg->long_name()) == ln_map_.end(), "Cmdline: Redefined long name");
        ln_map_[arg->long_name()] = arg;
    }
}

void Cmdline::register_submodule(Cmdline* sub, const std::string& name) {
    assert_msg(sub != nullptr, "Cmdline Internal Error!");

    assert_msg(sub_map_.find(name) == sub_map_.end(), "Cmdline: Redefined Submodule Name!");
    sub_map_[name] = sub;
}

struct print_context_t {
    list<Switch*> lswt;
    list<ArgumentBase*> larg, larg_must;
    list<SerialBase*> lser, lser_must;
    list<Action*> lact;
    list<Cmdline*> lsub;
};

static void classify__(print_context_t& pcxt, CmdBase* base, bool is_ln) {
    int typ = base->cmd_type();
    
    if (is_ln && base->short_name() != '\0') {
        if (typ == CmdType::SWITCH || typ == CmdType::ARGUMENT || typ == CmdType::ACTION)
            return;
    }

    if (typ == CmdType::SWITCH) {
        Switch* swt = dynamic_cast<Switch*>(base);
        pcxt.lswt.push_back(swt);
    } else if (typ == CmdType::ARGUMENT) {
        ArgumentBase* arg = dynamic_cast<ArgumentBase*>(base);
        if (arg->is_must())
            pcxt.larg_must.push_back(arg);
        else
            pcxt.larg.push_back(arg);
    } else if (typ == CmdType::SERIAL && !is_ln) { 
        // 在long name模式下统计Serial，Serial应该按index排序而不是long name
        SerialBase* ser = dynamic_cast<SerialBase*>(base);
        if (ser->is_must())
            pcxt.lser_must.push_back(ser);
        else
            pcxt.lser.push_back(ser);
    } else if (typ == CmdType::ACTION) {
        Action* act = dynamic_cast<Action*>(base);
        pcxt.lact.push_back(act);
    }
}

static void print_usage__(print_context_t& pcxt, const std::string& mod_list) {
    printf("Usage: ");
    printf("\t%s ", mod_list.c_str());

    // print serial
    for (auto it = pcxt.lser_must.begin(); it != pcxt.lser_must.end(); it++) {
        printf("<%s> ", (*it)->long_name().c_str());
    }

    // print must arg
    for (auto it = pcxt.larg_must.begin(); it != pcxt.larg_must.end(); it++) {
        if (!(*it)->long_name().empty())
            printf("--%s <%s> ", (*it)->long_name().c_str(), (*it)->long_name().c_str());
        else
            printf("--%c <%c> ", (*it)->short_name(), (*it)->short_name());
    }

    printf("\n\n");
}

static void print_des__(const std::string& des) {
    printf("%s\n\n", des.c_str());
}

static void print_submod__(print_context_t& pcxt) {
    if (pcxt.lsub.empty()) return;

    printf("Submodule:\n");
    for (auto it = pcxt.lsub.begin(); it != pcxt.lsub.end(); it++) {
        printf("\t%s  \t%s\n", (*it)->name().c_str(), (*it)->des().c_str());
    }
    printf("\n");
}

static void print_serial__(print_context_t& pcxt) {
    if (pcxt.lser.empty() && pcxt.lser_must.empty()) return;

    printf("Serial:\n");
    for (auto it = pcxt.lser_must.begin(); it != pcxt.lser_must.end(); it++) {
        printf("*\t%d: %s  \t%s\n", 
            (*it)->get_idx(),
            (*it)->long_name().c_str(), 
            (*it)->des().c_str()
        );
    }

    for (auto it = pcxt.lser.begin(); it != pcxt.lser.end(); it++) {
        printf("\t%d: %s  \t%s. default: '%s'\n", 
            (*it)->get_idx(),
            (*it)->long_name().c_str(), 
            (*it)->des().c_str(),
            (*it)->get_def().c_str()
        );
    }
    printf("\n");
}

static void print_argument__(print_context_t& pcxt) {
    if (pcxt.larg.empty() && pcxt.larg_must.empty()) return;

    printf("Argument:\n");
    for (auto it = pcxt.larg_must.begin(); it != pcxt.larg_must.end(); it++) {
        printf("*\t");
        if ((*it)->short_name() != '\0' && !(*it)->long_name().empty())
            printf("-%c, --%s", (*it)->short_name(), (*it)->long_name().c_str());
        else if ((*it)->short_name() != '\0') 
            printf("-%c", (*it)->short_name());
        else 
            printf("--%s", (*it)->long_name().c_str());
        printf("  \t%s\n", (*it)->des().c_str());
    }

    for (auto it = pcxt.larg.begin(); it != pcxt.larg.end(); it++) {
        printf("\t");
        if ((*it)->short_name() != '\0' && !(*it)->long_name().empty())
            printf("-%c, --%s", (*it)->short_name(), (*it)->long_name().c_str());
        else if ((*it)->short_name() != '\0') 
            printf("-%c", (*it)->short_name());
        else 
            printf("--%s", (*it)->long_name().c_str());
        printf("  \t%s. default: %s\n", (*it)->des().c_str(), (*it)->get_def().c_str());
    }
    printf("\n");
}

static void print_switch__(print_context_t& pcxt) {
    if (pcxt.lswt.empty()) return;

    printf("Switch (default: false):\n");
    for (auto it = pcxt.lswt.begin(); it != pcxt.lswt.end(); it++) {
        printf("\t");
        if ((*it)->short_name() != '\0' && !(*it)->long_name().empty())
            printf("-%c, --%s", (*it)->short_name(), (*it)->long_name().c_str());
        else if ((*it)->short_name() != '\0') 
            printf("-%c", (*it)->short_name());
        else 
            printf("--%s", (*it)->long_name().c_str());
        printf("  \t%s\n", (*it)->des().c_str());
    }
    printf("\n");
}

static void print_action__(print_context_t& pcxt) {
    if (pcxt.lact.empty()) return;

    printf("Action:\n");
    for (auto it = pcxt.lact.begin(); it != pcxt.lact.end(); it++) {
        printf("\t");
        if ((*it)->short_name() != '\0' && !(*it)->long_name().empty())
            printf("-%c, --%s", (*it)->short_name(), (*it)->long_name().c_str());
        else if ((*it)->short_name() != '\0') 
            printf("-%c", (*it)->short_name());
        else 
            printf("--%s", (*it)->long_name().c_str());
        printf("  \t%s\n", (*it)->des().c_str());
    }
    printf("\n");
}

std::string mod_list__(Cmdline* cur) {
    if (cur->parent() == nullptr)
        return cur->name();
    return mod_list__(cur->parent()) + " " + cur->name();
}

void Cmdline::print_help() {
    print_context_t pcxt;

    for (auto it = sn_map_.begin(); it != sn_map_.end(); it++) {
        classify__(pcxt, it->second, false);
    }

    for (auto it = ln_map_.begin(); it != ln_map_.end(); it++) {
        classify__(pcxt, it->second, true);
    }

    for (auto it = idx_map_.begin(); it != idx_map_.end(); it++) {
        classify__(pcxt, it->second, false);
    }

    for (auto it = sub_map_.begin(); it != sub_map_.end(); it++) {
        pcxt.lsub.push_back(it->second);
    }

    print_usage__(pcxt, mod_list__(this));
    print_des__(des_);
    print_submod__(pcxt);
    print_serial__(pcxt);
    print_argument__(pcxt);
    print_switch__(pcxt);
    print_action__(pcxt);
}

void Cmdline::print_error() {
    printf("Error: %s\n\n", err_msg_.str().c_str());
    print_help();
}

void Cmdline::print_internal_error() {
    printf("Internal Error!\n");
}

int Cmdline::parser(int argc, char** argv) {
    assert_msg(check_def__(), "Cmdline: Wrong define with serial index!");
    assert_msg(argv != nullptr, "Cmdline Internal Error!");
    change_active__();

    // there is not command line args
    if (argc <= 1) return CmdRetCode::SUCCESS;

    Cmdline* sub = check_submodule__(argv[1]);
    if (sub == nullptr) 
        return do_parser__(argc, argv);
    else 
        return sub->parser(argc - 1, argv + 1);
}

int Cmdline::run(int argc, char** argv) {
    assert_msg(argv != nullptr, "Cmdline Internal Error!");
    int r = parser(argc, argv);
    Cmdline* cur = active_module();
    if (r == CmdRetCode::SUCCESS) {
        if (cur->act_ != nullptr)
            return cur->act_->run(cur);
        return cur->def_run();
    } else if (r == CmdRetCode::FORMAT_ERROR) {
        if (cur->act_ && cur->act_->long_name() == "help")
            cur->lprint_help();
        else
            cur->lprint_error();
        return r;
    } else {
        cur->lprint_internal_error();
        return r;
    }
}

bool Cmdline::check_def__() {
    if (parent_ != nullptr)
        return true;
    if (idx_map_.size() != max_idx_)
        return false;
    int last = 0;
    int must = 0;
    for (auto it = idx_map_.begin(); it != idx_map_.end(); it++) {
        if (it->first == last)
            return false;
        if (it->second->is_must()) {
            if (it->first - must > 1)
                return false;
            must = it->first;
        }
        last = it->first;
    }
    return true;
}

Cmdline* Cmdline::check_submodule__(const std::string& str) {
    auto it = sub_map_.find(str);
    if (it != sub_map_.end())
        return it->second;
    return nullptr;
}

Cmdline* Cmdline::top_module__() {
    Cmdline* cur = this;
    while (cur->parent_ != nullptr)
        cur = cur->parent_;
    return cur;
}

void Cmdline::change_active__() {
    Cmdline* top = top_module__();
    top->active_ = this;
}

int Cmdline::do_parser__(int argc, char** argv) {
    int idx = 1;
    int serial_idx = 0;
    int must_done = 0;  // 已经设置的必选项数量
    while (idx < argc) {
        char* str = argv[idx];
        int len = strlen(str);
        if (len == 1 && str[0] == '-') {
            err_msg_ << "single '-' is invalid";
            return CmdRetCode::FORMAT_ERROR;
        } else if (len > 1 && str[0] == '-') {
            if (len == 2 && str[1] == '-') {
                err_msg_ << "single '--' is invalid";
                return CmdRetCode::FORMAT_ERROR;
            } else if (len > 2 && str[1] == '-') {
                // long name
                auto it = ln_map_.find(str + 2);
                if (it == ln_map_.end() || it->second->done()) {
                    err_msg_ << (str + 2) << " is not existed, or have been setted";
                    return CmdRetCode::FORMAT_ERROR;
                }
                int typ = it->second->cmd_type();
                if (typ == CmdType::SWITCH) {
                    Switch* swt = dynamic_cast<Switch*>(it->second);
                    swt->set(true);
                } else if (typ == CmdType::ARGUMENT) {
                    ArgumentBase* arg = dynamic_cast<ArgumentBase*>(it->second);
                    idx++;
                    if (idx >= argc) {
                        err_msg_ << "not match value with " << arg->long_name();
                        return CmdRetCode::FORMAT_ERROR;
                    }
                    char* val = argv[idx];
                    if (!arg->set_with_str(val)) {
                        err_msg_ << "set " << arg->long_name() << " with " << val << " faild";
                        return CmdRetCode::FORMAT_ERROR;
                    }
                    if (arg->is_must())
                        must_done++;
                } else if (typ == CmdType::ACTION) {
                    if (act_ == nullptr)
                        act_ = dynamic_cast<Action*>(it->second);
                    else {
                        err_msg_ << "not support actions mush than one";
                        return CmdRetCode::FORMAT_ERROR;
                    }
                } else {
                    err_msg_ << "please kill the developer";
                    return CmdRetCode::DEF_ERROR;
                }
                it->second->set_done();
            } else {
                // short name
                if (len == 2) {
                    // single define
                    auto it = sn_map_.find(str[1]);
                    if (it == sn_map_.end() || it->second->done()) {
                        err_msg_ << "short name (" << str[1] << ") is not existed, or have been setted";
                        return CmdRetCode::FORMAT_ERROR;
                    }
                    int typ = it->second->cmd_type();
                    if (typ == CmdType::SWITCH) {
                        Switch* swt = dynamic_cast<Switch*>(it->second);
                        swt->set(true);
                    } else if (typ == CmdType::ARGUMENT) {
                        ArgumentBase* arg = dynamic_cast<ArgumentBase*>(it->second);
                        idx++;
                        if (idx >= argc) {
                            err_msg_ << "not match value with " << arg->long_name() << " (" << str[1] << ")";
                            return CmdRetCode::FORMAT_ERROR;
                        }
                        char* val = argv[idx];
                        if (!arg->set_with_str(val)) {
                            err_msg_ << "set " << arg->long_name() << " with " << val << " faild";
                            return CmdRetCode::FORMAT_ERROR;
                        }
                        if (arg->is_must())
                            must_done++;
                    } else if (typ == CmdType::ACTION) {
                        if (act_ == nullptr)
                            act_ = dynamic_cast<Action*>(it->second);
                        else  {
                            err_msg_ << "not support actions mush than one";
                            return CmdRetCode::FORMAT_ERROR;
                        }
                    } else 
                        return CmdRetCode::DEF_ERROR;
                    it->second->set_done();
                } else {
                    // multi define
                    for (int ci = 1; ci < len; ci++) {
                        auto it = sn_map_.find(str[ci]);
                        if (it == sn_map_.end() || it->second->done()) {
                            err_msg_ << "short name (" << str[1] << ") is not existed, or have been setted";
                            return CmdRetCode::FORMAT_ERROR;
                        }
                        int typ = it->second->cmd_type();
                        if (typ == CmdType::SWITCH) {
                            Switch* swt = dynamic_cast<Switch*>(it->second);
                            swt->set(true);
                        } else if (typ == CmdType::ACTION) {
                            if (act_ == nullptr)
                                act_ = dynamic_cast<Action*>(it->second);
                            else  {
                                err_msg_ << "not support actions mush than one";
                                return CmdRetCode::FORMAT_ERROR;
                            }
                        } else if (typ == CmdType::ARGUMENT) {
                            err_msg_ << "can't define multi short name with other value";
                            return CmdRetCode::FORMAT_ERROR;
                        } else {
                            err_msg_ << "please kill the developer";
                            return CmdRetCode::DEF_ERROR;
                        }
                        it->second->set_done();
                    }
                }
            }
        } else if (serial_idx >= max_idx_) {
            if (tail_) {
                tail_vec_.push_back(str);
            } else {
                err_msg_ << "not matched serial";
                return CmdRetCode::FORMAT_ERROR;
            }
        } else {
            // serial
            serial_idx++;
            auto it = idx_map_.find(serial_idx);
            printf("hello!\n");
            if (it == idx_map_.end() || it->second->done()) {
                err_msg_ << "not matched serial";
                return CmdRetCode::FORMAT_ERROR;
            }
            if (!it->second->set_with_str(str)) {
                err_msg_ << "set " << it->second->long_name() << " with " << str << " faild";
                return CmdRetCode::FORMAT_ERROR;
            }
            if (it->second->is_must())
                must_done++;
            it->second->set_done();
        }

        idx++;
    }

    // check
    int must_need = 0;  // 需要被标记的必选项
    // 所有项都需要<ln>，所以查询ln_map_就可以了
    for (auto it = ln_map_.begin(); it != ln_map_.end(); it++) {
        if (it->second->cmd_type() == CmdType::ARGUMENT) {
            ArgumentBase* arg = dynamic_cast<ArgumentBase*>(it->second);
            if (arg->is_must())
                must_need++;
        }
    }

    for (auto it = idx_map_.begin(); it != idx_map_.end(); it++) {
        if (it->second->is_must())
            must_need++;
    }

    if (must_done != must_need) {
        err_msg_ << "all * item must be setted";
        return CmdRetCode::FORMAT_ERROR;
    }

    return CmdRetCode::SUCCESS;
}

template<>
bool def_trans_func<std::string>(std::string& val, const std::string& str) {
    val = str;
    return true;
}

template<>
bool def_trans_func<int>(int& val, const std::string& str) {
    val = stoi(str);
    return true;
}

template<>
bool def_trans_func<long>(long& val, const std::string& str) {
    val = stol(str);
    return true;
}

template<>
bool def_trans_func<unsigned long>(unsigned long& val, const std::string& str) {
    val = stoul(str);
    return true;
}

template<>
bool def_trans_func<long long>(long long& val, const std::string& str) {
    val = stoll(str);
    return true;
}

template<>
bool def_trans_func<unsigned long long>(unsigned long long& val, const std::string& str) {
    val = stoull(str);
    return true;
}

template<>
bool def_trans_func<float>(float& val, const std::string& str) {
    val = stof(str);
    return true;
}

template<>
bool def_trans_func<double>(double& val, const std::string& str) {
    val = stod(str);
    return true;
}

template<>
bool def_trans_func<long double>(long double& val, const std::string& str) {
    val = stold(str);
    return true;
}

} // namespace cli
} // namespace flame