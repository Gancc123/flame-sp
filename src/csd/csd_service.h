#ifndef FLAME_CSD_SERVICE_H
#define FLAME_CSD_SERVICE_H

#include "csd_context.h"

namespace flame {

class CsdService {
public:
    virtual ~CsdService() {}

protected:
    CsdService(CsdContext* cct) : cct_(cct), fct_(cct->fct()) {}

    CsdContext* cct_;
    FlameContext* fct_;
}; // class CsdService 

} // namespace flame

#endif // FLAME_MGR_SERVICE_H