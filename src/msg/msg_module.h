#ifndef FLAME_MSG_MSG_MODULE_H
#define FLAME_MSG_MSG_MODULE_H

namespace flame{

class MsgConfig;
class MsgManager;
class FlameContext;
class MsgerCallback;

struct MsgModule{
    MsgConfig *config;
    MsgManager *manager;
};

int msg_module_init(FlameContext *fct, MsgerCallback *msger_cb,
                                                    MsgConfig *config=nullptr);

int msg_module_finilize(FlameContext *fct);

}

#endif