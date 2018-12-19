#ifndef FLAME_MSG_INTERNAL_REF_COUNTED_OBJ_H
#define FLAME_MSG_INTERNAL_REF_COUNTED_OBJ_H

#include "common/context.h"
#include "msg/msg_def.h"

#include <atomic>
#include <cassert>

namespace flame{

struct RefCountedObject {
private:
    mutable std::atomic<uint64_t> nref;
protected:
    FlameContext *fct;
public:
    RefCountedObject(FlameContext *c = NULL, uint64_t n=1) : nref(n), fct(c) {}
    virtual ~RefCountedObject() {
        //assert(nref == 0);
    }
    
    const RefCountedObject *get() const {
        uint64_t v = ++nref;
        MLI(fct, trace, "RefCountedObject::get {}->{} {}",
                                                v-1, v, typeid(*this).name());
        return this;
    }
    RefCountedObject *get() {
        uint64_t v = ++nref;
        MLI(fct, trace, "RefCountedObject::get {}->{} {}",
                                                v-1, v, typeid(*this).name());
        return this;
    }
    void put() const {
        uint64_t v = --nref;
        MLI(fct, trace, "RefCountedObject::put {}->{} {}",
                                                v+1, v, typeid(*this).name());
        if (v == 0) {
            delete this;
        } 
    }
    void set_fct(FlameContext *c) {
        fct = c;
    }

    FlameContext *get_fct() const{
        return fct;
    }

    uint64_t get_nref() const {
        return nref;
    }
};

}

#endif