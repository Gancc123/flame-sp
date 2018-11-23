#ifndef FLAME_MGR_H
#define FLAME_MGR_H

#include "common/context.h"
#include "metastore/ms.h"

namespace flame {

class MGR {
public:
    MGR(FlameContext* fct__) : fct(fct__) {}
    ~MGR() {}

    int run_server();
private:
    FlameContext* fct;
};

} // namespace flame

#endif // FLAME_MGR_H