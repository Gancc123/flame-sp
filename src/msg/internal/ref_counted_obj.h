#ifndef FLAME_MSG_INTERNAL_REF_COUNTED_OBJ_H
#define FLAME_MSG_INTERNAL_REF_COUNTED_OBJ_H

#include "msg/msg_context.h"
#include "msg/msg_def.h"

#include <atomic>
#include <cassert>

namespace flame{
namespace msg{

struct RefCountedObject {
private:
    mutable std::atomic<uint64_t> nref;
protected:
    MsgContext *mct;
public:
    RefCountedObject(MsgContext *c=NULL, uint64_t n=1) : nref(n), mct(c) {}
    virtual ~RefCountedObject() {
        //assert(nref == 0);
    }
    
    const RefCountedObject *get() const {
        uint64_t v = ++nref;
        MLI(mct, trace, "RefCountedObject::get {}->{} {}",
                                                v-1, v, typeid(*this).name());
        return this;
    }
    RefCountedObject *get() {
        uint64_t v = ++nref;
        MLI(mct, trace, "RefCountedObject::get {}->{} {}",
                                                v-1, v, typeid(*this).name());
        return this;
    }
    void put() const {
        uint64_t v = --nref;
        MLI(mct, trace, "RefCountedObject::put {}->{} {}",
                                                v+1, v, typeid(*this).name());
        if (v == 0) {
            delete this;
        } 
    }
    void set_mct(MsgContext *c) {
        mct = c;
    }

    MsgContext *get_mct() const{
        return mct;
    }

    uint64_t get_nref() const {
        return nref;
    }
};

} //namespace msg
} //namespace flame

#endif