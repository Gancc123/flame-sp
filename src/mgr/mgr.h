#ifndef FLAME_MGR_H
#define FLAME_MGR_H

#include "common/context.h"
#include "metastore/ms.h"

#include "mgr_context.h"
#include "mgr_server.h"

#include <memory>

namespace flame {

class MGR {
public:
    MGR(FlameContext* fct__) : fct(fct__) {}
    ~MGR() {}


    int run_server();
private:
    FlameContext* fct;
    MgrContext* mct;
    std::unique_ptr<MgrServer> server_;
};

} // namespace flame

#endif // FLAME_MGR_H