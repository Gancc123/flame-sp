#ifndef FLAME_COMMON_ATOM_QUEUE_H
#define FLAME_COMMON_ATOM_QUEUE_H

#include <cassert>
#include <atomic>
#include <cstdio>
#include <queue>

namespace flame {

/**
 * @brief 原子队列接口
 * 
 * @tparam ELEM 
 */
template<typename ELEM>
class AtomQueue {
public:
    virtual ~AtomQueue() {}

    virtual bool empty() const = 0;

    virtual bool push(const ELEM&) = 0;

    virtual bool pop(ELEM&) = 0;

protected:
    AtomQueue() {}
}; // class AtomQueue

/**
 * @brief 链式无锁队列
 * 基于单向环形链表实现
 * - 不需要分配固定内存空间，故而元素数量不设上限
 * - 由于每次元素都需要new，性能可能受此影响
 * 
 * @tparam ELEM 
 */
template<typename ELEM>
class AtomLinkedQueue : public AtomQueue<ELEM> {
public:
    explicit AtomLinkedQueue() {}

    ~AtomLinkedQueue() {}

    virtual bool empty() const override { return &head_ == head_.next; }

    virtual bool push(const ELEM& elem) override {
        AtomEntry* none = &head_;
        AtomEntry* tail;
        AtomEntry* entry = new AtomEntry(elem, none);
        assert(entry != nullptr);

        while (true) {
            tail = tail_;
            while (tail->next != none)
                tail = tail->next;
            
            if (tail->next.compare_exchange_weak(none, entry))
                break;
            
            none = &head_; // 失败时, none的值会被修改
        }
        
        AtomEntry* origin = tail;
        while (!tail_.compare_exchange_weak(origin, entry))
            origin = tail;
        return true;
    }

    virtual bool pop(ELEM& elem) override { 
        AtomEntry* origin = nullptr;
        AtomEntry* entry = nullptr;
        AtomEntry* next = nullptr;

        do {
            entry = head_.next;
            if (entry == &head_) // 队列为空
                return false;
            next = entry->next;
        } while (!head_.next.compare_exchange_weak(entry, next));

        do {
            origin = entry;
            next = entry->next;
            if (tail_.load() != entry)
                break;
        } while (!tail_.compare_exchange_weak(origin, next));

        elem = entry->elem;
        delete entry;
        return true;
    }

    AtomLinkedQueue(const AtomLinkedQueue&) = delete;
    AtomLinkedQueue(AtomLinkedQueue&&) = delete;
    AtomLinkedQueue& operator = (const AtomLinkedQueue&) = delete;
    AtomLinkedQueue& operator = (AtomLinkedQueue&&) = delete;

private:
    struct AtomEntry {
        explicit AtomEntry(const ELEM& e, AtomEntry* n) : elem(e), next(n) {}
        explicit AtomEntry(AtomEntry* n) : next(n) {}

        ELEM elem;
        std::atomic<AtomEntry*> next {nullptr};
    }; // struct AtomEntry

    AtomEntry head_ {&head_};   // 单向环形链表
    std::atomic<AtomEntry*> tail_ {&head_};
}; // class AtomLinkedQueue

/**
 * @brief 环形原子队列
 * 基于数组环实现
 * - 需要一开始指定队列大小
 * 
 * @tparam ELEM 
 */
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

/**
 * @brief 带随机访问的环形原子队列
 * RandomRingQueue把环形队列分为两部分：
 * - 队列访问区域：FIFA的访问，被访问的（load）的元素加入随机访问区域
 * - 随机访问区域：允许随机访问（包括删除，trim）
 * 
 * @tparam ELEM 
 */
template<typename ELEM>
class RandomRingQueue : public AtomQueue<ELEM> {
public:
    explicit RandomRingQueue(size_t cap, size_t max_try_times = 0)
    : cap_(cap), max_try_times_(max_try_times) {
        arr_ = alloc__(cap_);
    }

    ~RandomRingQueue() { free__(arr_); }

    void set_max_try_times(size_t t = 0) { max_try_times_ = t; }
    bool max_try_times() const { return max_try_times_; }

    size_t capacity() const { return cap_; }

    size_t size() const { return (cap_ + prod_head_.load() - trim_tail_.load()) % cap_; }
    bool empty() const { return size() == 0; }
    bool full() const { return size() == capacity(); }

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

    void trim() { while (trim__()); }

    bool remove_trim(size_t idx) {
        bool r = remove(idx);
        trim();
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

    RandomRingQueue(const RandomRingQueue&) = delete;
    RandomRingQueue(RandomRingQueue&&) = delete;
    RandomRingQueue& operator = (const RandomRingQueue&) = delete;
    RandomRingQueue& operator = (RandomRingQueue&&) = delete;

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
    std::atomic<uint64_t> prod_head_ {0};
    std::atomic<uint64_t> prod_tail_ {0};
    std::atomic<uint64_t> load_tail_ {0};
    std::atomic<uint64_t> load_head_ {0};
    std::atomic<uint64_t> trim_head_ {0};
    std::atomic<uint64_t> trim_tail_ {0};

    size_t max_try_times_ {0};

    size_t save__(ELEM* elem, bool block) {
        assert(elem != nullptr);
        int try_times = 0;
        uint64_t prod_head;
        uint64_t prod_next;
        uint64_t trim_tail;
        while (true) {
            prod_head = prod_head_;
            prod_next = prod_head + 1;
            trim_tail = trim_tail_;
            if (prod_next - trim_tail > cap_) {
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
        size_t idx = count_index__(prod_head);
        arr_[idx].elem = elem;
        arr_[idx].turn_save();
        uint64_t origin = prod_head;
        while (!prod_tail_.compare_exchange_weak(origin, prod_next))
            origin = prod_head;
        return idx;
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
            load_next = load_head + 1;
            if (load_head >= prod_tail) {
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
        size_t idx = count_index__(load_head);
        *elem = arr_[idx].elem;
        arr_[idx].turn_load();

        uint64_t origin = load_head;
        while (!load_tail_.compare_exchange_weak(origin, load_next))
            origin = load_head;
        return idx;
    }

    bool trim__() {
        Entry* entry;
        size_t load_tail;
        size_t trim_head;
        size_t trim_next;
        while (true) {
            load_tail = load_tail_;
            trim_head = trim_head_;
            trim_next = trim_head + 1;
            if (trim_head >= load_tail) return false; // empty
        
            entry = arr_ + count_index__(trim_head);
            if (entry->is_load() && entry->is_save()) return false;   // the item can't be trimed

            if (trim_head_.compare_exchange_weak(trim_head, trim_next))
                break;
        }
        if (entry->is_trim()) {
            entry->clear();
            entry->turn_none();
        }

        uint64_t origin = trim_head;
        while (!trim_tail_.compare_exchange_weak(origin, trim_next))
            origin = trim_head;
        return true;
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
        if (idx >= cap_) return nullptr;
        return arr_ + idx;
    }

    size_t count_index__(uint64_t i) { return i % cap_; }

}; // class RandomRingQueue

} // namespace flame

#endif // !FLAME_COMMON_ATOM_QUEUE_H