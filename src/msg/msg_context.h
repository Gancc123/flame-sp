#ifndef FLAME_MSG_MSG_CONTEXT_H
#define FLAME_MSG_MSG_CONTEXT_H

#include "include/flame.h"
#include "common/context.h"

namespace flame{
namespace msg{

class MsgConfig;
class MsgManager;
class MsgerCallback;
class MsgDispatcher;

class CsdAddrResolver{
public:
    virtual int pull_csd_addr(std::list<csd_addr_t>& res,
                                 const std::list<uint64_t>& csd_id_list) = 0;
};

struct MsgContext{
    FlameContext *fct;
    MsgConfig *config;
    MsgManager *manager;
    MsgDispatcher *dispatcher;
    CsdAddrResolver *csd_addr_resolver;

    explicit MsgContext(FlameContext *c)
    : fct(c), config(nullptr), manager(nullptr), csd_addr_resolver(nullptr),
      dispatcher(nullptr) {};

    int load_config();

    int init(MsgerCallback *msger_cb, CsdAddrResolver *r);
    int fin();
};

} //namespace msg
} //namespace flame

#endif