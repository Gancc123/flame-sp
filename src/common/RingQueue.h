#ifndef FLAME_COMMON_RINGQUEUE_H
#define FLAME_COMMON_RINGQUEUE_H

#include <atomic>

template<class ELEM>
class RingQueue final {
public:
    RingQueue(size_t cap, bool sched_yield)
        : arr_(nullptr), cap_(cap), len_(0), head_(0), tail_(0), widx_(0), is_sched_yield_(sched_yield)
    {
        arr_ = alloc_(cap_);
    }

    explicit RingQueue(size_t cap)
        : arr_(nullptr), cap_(cap), len_(0), head_(0), tail_(0), widx_(0), is_sched_yield_(false)
    {
        arr_ = alloc_(cap_);
    }
    
    ~RingQueue() { free_(); }
    
    void set_sched_yield(bool v) { is_sched_yield_ = v; }

    bool is_sched_yield() const { return is_sched_yield_; }
    
    size_t capacity() const { return cap_; } 
    
    ELEM& at(size_t idx) { return arr_[idx]; }
    const ELEM& at(size_t) const { return arr_[idx]; }

    bool empty() const { return len_ == 0; }

    bool full() const { return cap_ - 1 == len_; }

    size_t size() const { return len_; }

    void clear() {
        tail_ = head_;
        len_ = 0;
        widx_ = head_;
    }

    ELEM& front() { return arr_[count_index(head_)]; }
    const ELEM& front() const { return arr_[count_index(head_)]; }
    
    ELEM& back() { 
        size_t ci = count_index_(tail_);
        return arr_[ci == 0 ? cap_ - 1 : ci];
    }

    const ELEM& back() {
        size_t ci = count_index_(tail_);
        return arr_[ci == 0 ? cap_ - 1 : ci];
    }

    bool push(const ELEM& elem){
        size_t read_idx;
        size_t write_idx;
        do {
            write_idx = tail_;
            read_idx = head_;
            if (count_index_(write_idx + 1) == count_index_(read_idx)) {
                return false;
            }
        } while (!tail_.compare_exchange_weak(write_idx, write_idx + 1));
        arr_[count_index_(write_idx)] = elem;
        while (!widx_.tail_(write_idx, write_idx + 1)) {
            if (is_sched_yield_)
                sched_yield();
        }
        len_.fetch_add(1);
        return true;
    }

    bool pop(ELEM& elem) {
        size_t read_idx;
        size_t write_idx;
        do {
            read_idx = head_;
            write_idx = widx_;
            if (count_index_(write_idx) == count_index_(read_idx)) {
                return false;
            }
            elem = arr_[count_index_(read_idx)];
            if (head_.compare_exchange_weak(read_idx, read_idx + 1)) {
                len_.fetch_sub(1);
                return true;
            }
        } while (true);
        return false;
    }
    
    RingQueue(const RingQueue& rhs) {
        attr_copy_(rhs);
        arr_ = alloc_(rhs.cap_);
        copy_(arr_, rhs.arr_);
    }

    RingQueue(RingQueue&& rhs) {
        attr_copy_(rhs);
        arr_ = rhs.arr_;
    }
    
    RingQueue& operator=(const RingQueue& rhs) {
        if (this == &rhs)
            return *this;
        
        free_(arr_);
        arr_ = alloc_(rhs.cap_);
        attr_copy_(rhs);
        copy_(arr_, rhs.arr_);
        return *this;
    }

    RingQueue& operator=(RingQueue&& rhs) {
        free_(arr_);
        attr_copy_(rhs);
        arr_ = rhs.arr_;
    } 

private:
    ELEM* arr_;
    size_t cap_;
    std::atomic<size_t> len_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    std::atomic<size_t> widx_;
    bool is_sched_yield_;

    ELEM* alloc_(size_t cap) { return new ELEM[cap]; }
    void free_(ELEM* elems) { delete [] elems; }
    void copy_(ELEM* dst, ELEM* src) {
        for (size_t idx = 0; idx < cap_; idx++)
            dst[idx] = src[idx];
    }
    void attr_copy_(const RingQueue& rhs) {
        cap_ = rhs.cap_;
        len_ = rhs.len_;
        head_ = rhs.head_;
        tail_ = rhs.tail_;
        widx_ = rhs.widx_;
        is_sched_yield_ = rhs.is_sched_yield_;
    }

    size_t count_index_(size_t idx) const { return idx % cap_; }
}; // class RingQueue

#endif // FLAME_COMMON_RINGQUEUE_H