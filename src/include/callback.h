#ifndef FLAME_INCLUDE_CALLBACK_H
#define FLAME_INCLUDE_CALLBACK_H

namespace flame {

typedef void (*callback_fn_t)(int, void* arg1, void* arg2);

struct callback_t {
    callback_fn_t cb_fn;
    void* arg1;
    void* arg2;
};

} // namespace flame

#endif // !FLAME_INCLUDE_CALLBACK_H
