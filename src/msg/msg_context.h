#ifndef FLAME_MSG_MSG_CONTEXT_H
#define FLAME_MSG_MSG_CONTEXT_H

#include "common/context.h"

namespace flame{

class MsgConfig;
class MsgManager;
class MsgerCallback;

struct MsgContext{
    FlameContext *fct;
    MsgConfig *config;
    MsgManager *manager;

    explicit MsgContext(FlameContext *c)
    : fct(c), config(nullptr), manager(nullptr) {};

    int load_config();

    int init(MsgerCallback *msger_cb);
    int fin();
};

}

#endif