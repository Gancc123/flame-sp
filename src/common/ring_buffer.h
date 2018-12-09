#ifndef FLAME_COMMON_RING_BUFFER_H
#define FLAME_COMMON_RING_BUFFER_H

#include <cassert>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <sched.h>

namespace flame {

template<typename ELEM>
class RingBuffer final {
public:
    explicit RingBuffer(size_t cap, size_t max_try_times = 0)
    : cap_(cap), real_cap_(cap - 1), max_try_times_(max_try_times) {
        arr_ = alloc__(cap_);
    }

    ~RingBuffer() { free__(arr_); }

    void set_max_try_times(size_t t = 0) { max_try_times_ = t; }
    bool max_try_times() const { return max_try_times_; }

    size_t capacity() const { return cap_; }
    size_t real_cap() const { return real_cap_; }

    size_t size() const { return (cap_ + prod_head_.load() - trim_tail_.load()) % cap_; }
    bool empty() const { return size() == 0; }
    bool full() const { return size() == real_cap(); }

    size_t end() const { return cap_; }

    ELEM* get(size_t idx) {
        Entry* entry = get_entry__(idx);
        if (entry && entry->is_load())
            return entry->elem;
        return nullptr;
    }

    bool remove(size_t idx) {
        Entry* entry = get_entry__(idx);
        if (entry && !entry->is_save()) {
            if (entry->is_load())
                entry->turn_trim();
            return true;
        }
        return false;
    }

    void trim() { trim__(); }

    bool remove_trim(size_t idx) {
        bool r = remove(idx);
        trim__();
        return r;
    }

    size_t save(ELEM* elem) { return save__(elem, true); }
    size_t try_save(ELEM* elem) { return save__(elem, false); }
    size_t load(ELEM** elem) { return load__(elem, true); }
    size_t try_load(ELEM* elem) { return load__(elem, false); }

    bool push(const ELEM& elem) { return push__(elem, true); }
    bool try_push(const ELEM& elem) { return push__(elem, false); }
    bool pop(ELEM& elem) { return pop__(elem, true); }
    bool try_pop(ELEM& elem) { return pop__(elem, false); }

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator = (const RingBuffer&) = delete;
    RingBuffer& operator = (RingBuffer&&) = delete;

private:
    enum EntryStat {
        ENTRY_STAT_NONE = 0x0,  // none
        ENTRY_STAT_SAVE = 0x1,  // saved an valid elem
        ENTRY_STAT_LOAD = 0x2,  // elem can be load, elem is valid
        ENTRY_STAT_TRIM = 0x3   // wait for erase
    };

    struct Entry {
        Entry() {}
        ~Entry() { clear(); }

        ELEM* elem {nullptr};
        uint8_t stat {0};

        void turn_none() { stat = ENTRY_STAT_NONE; }
        bool is_none() const { return stat == ENTRY_STAT_NONE; }

        void turn_save() { stat = ENTRY_STAT_SAVE; }
        bool is_save() const { return stat == ENTRY_STAT_SAVE; }

        void turn_load() { stat = ENTRY_STAT_LOAD; }
        bool is_load() const { return stat == ENTRY_STAT_LOAD; }

        void turn_trim() { stat = ENTRY_STAT_TRIM; }
        bool is_trim() const { return stat == ENTRY_STAT_TRIM; }

        void clear() { delete elem; }
    }; // class Entry

    Entry* arr_ {nullptr};
    size_t cap_ {0};
    size_t real_cap_ {0};
    std::atomic<size_t> prod_head_ {0};
    std::atomic<size_t> prod_tail_ {0};
    std::atomic<size_t> load_tail_ {0};
    std::atomic<size_t> load_head_ {0};
    std::atomic<size_t> trim_head_ {0};
    std::atomic<size_t> trim_tail_ {0};

    size_t max_try_times_ {0};

    size_t save__(ELEM* elem, bool block) {
        assert(elem != nullptr);
        int try_times = 0;
        size_t prod_head;
        size_t prod_next;
        size_t trim_tail;
        while (true) {
            prod_head = prod_head_;
            prod_next = prod_head == real_cap_ ? 0 : prod_head + 1;
            trim_tail = trim_tail_;
            if (prod_next == trim_tail) {
                // full
                if (!block) 
                    return end();
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
        arr_[prod_head].elem = elem;
        arr_[prod_head].turn_save();
        while (!prod_tail_.compare_exchange_weak(prod_head, prod_next));
        return prod_head;
    }

    size_t load__(ELEM** elem, bool block) {
        assert(elem != nullptr);
        int try_times = 0;
        size_t prod_tail;
        size_t load_head;
        size_t load_next;
        while (true) {
            prod_tail = prod_tail_;
            load_head = load_head_;
            load_next = load_head == real_cap_ ? 0 : load_head + 1;
            if (load_head == prod_tail) {
                // emtpy
                if (!block)
                    return end();
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

            if (load_head_.compare_exchange_weak(load_head, load_next))
                break;
        }
        *elem = arr_[load_head].elem;
        arr_[load_head].turn_load();
        while (!load_tail_.compare_exchange_weak(load_head, load_next));
        return load_head;
    }

    void trim__() {
        Entry* entry;
        size_t load_tail;
        size_t trim_head;
        size_t trim_next;
        while (true) {
            load_tail = load_tail_;
            trim_head = trim_head_;
            trim_next = trim_head == real_cap_ ? 0 : trim_head + 1;
            if (trim_head == load_tail) return; // empty

            entry = arr_ + trim_head;
            if (entry->is_load() && entry->is_save()) return;   // the item can't be trimed

            if (trim_head_.compare_exchange_weak(trim_head, trim_next))
                break;
        }
        if (entry->is_trim()) {
            entry->clear();
            entry->turn_none();
        }
        while (!trim_tail_.compare_exchange_weak(trim_head, trim_next));
    }

    bool push__(const ELEM& elem, bool block) {
        ELEM* tmp = new ELEM(elem);
        size_t idx = save__(tmp, block);
        if (idx == end()) {
            delete tmp;
            return false;
        }
        return true;
    }

    bool pop__(ELEM& elem, bool block) {
        ELEM* tmp;
        size_t idx = load__(&tmp, block);
        if (idx == end()) return false;
        elem = *tmp;
        remove_trim(idx);
        return true;
    }

    Entry* alloc__(size_t cap) { return new Entry[cap]; }
    void free__(Entry* elems) { delete [] elems; }
    Entry* get_entry__(size_t idx) {
        if (idx > real_cap_) return nullptr;
        return arr_ + idx;
    }

}; // class RingBuffer

} // namespace flame

#endif // FLAME_COMMON_RING_BUFFER_H