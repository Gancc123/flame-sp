#ifndef FLAME_COMMON_RING_QUEUE_H
#define FLAME_COMMON_RING_QUEUE_H

#include "common/atom_queue.h"

#include <cstdio>

#include <cstdint>
#include <cstddef>
#include <atomic>

#include <sched.h>

namespace flame {

template<typename ELEM>
class RingQueue : public AtomQueue<ELEM> {
public:
    explicit RingQueue(size_t cap, size_t max_try_times = 0)
    : cap_(cap), max_try_times_(max_try_times) {
        arr_ = alloc__(cap_);
    }

    ~RingQueue() { free__(arr_); }

    void set_max_try_times(size_t t = 0) { max_try_times_ = t; }
    bool max_try_times() const { return max_try_times_; }

    size_t capacity() const { return cap_; }

    size_t size() const { return (cap_ + prod_head_.load() - cons_tail_.load()) % cap_; }
    virtual bool empty() const override { return size() == 0; }
    bool full() const { return size() == capacity(); }

    void clear() {
        cons_head_ = 0;
        cons_tail_ = 0;
        prod_head_ = 0;
        prod_tail_ = 0;
    }
    
    virtual bool push(const ELEM& elem) override { return push__(elem, true); }
    bool try_push(const ELEM& elem) { return push__(elem, false); }
    virtual bool pop(ELEM& elem) override { return pop__(elem, true); }
    bool try_pop(ELEM& elem) { return pop__(elem, false); }

    RingQueue(const RingQueue& rhs) {
        attr_copy__(rhs);
        arr_ = alloc__(cap_);
        copy__(arr_, rhs.arr_);
    }

    RingQueue(RingQueue&& rhs) {
        attr_copy__(rhs);
        arr_ = rhs.arr_;
    }

    RingQueue& operator = (const RingQueue& rhs) {
        if (this == &rhs)
            return *this;
        
        free__(arr_);
        arr_ = alloc__(rhs.cap_);
        attr_copy__(rhs);
        copy__(arr_, rhs.arr_);
        return *this;
    }

    RingQueue& operator = (RingQueue&& rhs) {
        free__(arr_);
        attr_copy__(rhs);
        arr_ = rhs.arr_;
    }

private:
    ELEM* arr_ {nullptr};
    size_t cap_ {0};
    std::atomic<uint64_t> cons_head_ {0};
    std::atomic<uint64_t> cons_tail_ {0};
    std::atomic<uint64_t> prod_head_ {0};
    std::atomic<uint64_t> prod_tail_ {0};
    size_t max_try_times_ {0};

    bool push__(const ELEM& elem, bool block) {
        int try_times = 0;
        uint64_t prod_head;
        uint64_t prod_next;
        uint64_t cons_tail;
        while (true) {
            prod_head = prod_head_;
            prod_next = prod_head + 1;
            cons_tail = cons_tail_;

            if (prod_next - cons_tail > cap_) {
                // full
                if (!block) 
                    return false;
                else if (max_try_times_ > 0) {
                    if (try_times < max_try_times_)
                        try_times++;
                    else {
                        sched_yield();
                        try_times = 0;
                    }
                } else
                    continue;
            }

            if (prod_head_.compare_exchange_weak(prod_head, prod_next))
                break;
        }
        arr_[count_index__(prod_head)] = elem;
        uint64_t origin = prod_head;
        while (!prod_tail_.compare_exchange_weak(origin, prod_next))
            origin = prod_head;
        return true;
    }

    bool pop__(ELEM& elem, bool block) {
        int try_times = 0;
        size_t prod_tail;
        size_t cons_head;
        size_t cons_next;
        while (true) {
            prod_tail = prod_tail_;
            cons_head = cons_head_;
            cons_next = cons_head + 1;

            if (cons_head >= prod_tail) {
                // emtpy
                if (!block)
                    return false;
                else if (max_try_times_ > 0) {
                    if (try_times < max_try_times_)
                        try_times++;
                    else {
                        sched_yield();
                        try_times = 0;
                    }
                } else
                    continue;
            }

            if (cons_head_.compare_exchange_weak(cons_head, cons_next))
                break;
        }
        elem = arr_[count_index__(cons_head)];
        uint64_t origin = cons_head;
        while (!cons_tail_.compare_exchange_weak(cons_head, cons_next))
            origin = cons_head;
        return true;
    }

    ELEM* alloc__(size_t cap) { return new ELEM[cap]; }
    void free__(ELEM* elems) { delete [] elems; }
    void copy__(ELEM* dst, ELEM* src, size_t len) {
        for (size_t i = 0; i < len; i++)
            dst[i] = src[i];
    }
    void attr_copy__(const RingQueue& rhs) {
        cap_ = rhs.cap_;
        cons_head_ = rhs.cons_head_;
        cons_tail_ = rhs.cons_tail_;
        prod_head_ = rhs.prod_head_;
        prod_tail_ = rhs.prod_tail_;
        max_try_times_ = rhs.max_try_times_;
    }

    size_t count_index__(uint64_t i) { return i % cap_; }

}; // class RingQueue

} // namespace flame

#endif // FLAME_COMMON_RING_QUEUE_H