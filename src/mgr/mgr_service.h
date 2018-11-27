#ifndef FLAME_MGR_SERVICE_H
#define FLAME_MGR_SERVICE_H

#include "mgr_context.h"

namespace flame {

class MgrService {
public:
    virtual ~MgrService() {}

protected:
    MgrService(MgrContext* mct) : mct_(mct), fct_(mct->fct()) {}

    MgrContext* mct_;
    FlameContext* fct_;
}; // class MgrService 

} // namespace flame

#endif // FLAME_MGR_SERVICE_H