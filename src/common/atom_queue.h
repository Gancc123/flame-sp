#ifndef FLAME_COMMON_ATOM_QUEUE_H
#define FLAME_COMMON_ATOM_QUEUE_H

#include <cassert>
#include <atomic>
#include <cstdio>
#include <queue>

namespace flame {

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

template<typename ELEM>
class AtomLinkedQueue : public AtomQueue<ELEM> {
public:
    explicit AtomLinkedQueue() {}

    ~AtomLinkedQueue() {}

    virtual bool empty() const override { return head_.next == nullptr; }

    virtual bool push(const ELEM& elem) override {
        AtomEntry* none = nullptr;
        AtomEntry* tail;
        AtomEntry* entry = new AtomEntry(elem);
        assert(entry != nullptr);

        while (true) {
            tail = tail_;
            while (tail && tail->next != nullptr)
                tail = tail->next;
            if (tail == nullptr || tail->next == tail)
                continue;
            
            none = nullptr;
            if (tail->next.compare_exchange_weak(none, entry))
                break;
        }
        
        AtomEntry* origin = tail;
        while (!tail_.compare_exchange_weak(origin, entry))
            origin = tail;
        return true;
    }

    virtual bool pop(ELEM& elem) override { 
        AtomEntry* none = nullptr;
        AtomEntry* origin = nullptr;
        AtomEntry* entry = nullptr;
        AtomEntry* next = nullptr;

        do {
            entry = head_.next;
            if (entry == nullptr)
                return false;
            next = entry->next;
        } while (!head_.next.compare_exchange_weak(entry, next));

        AtomEntry* tail;
        while (next == nullptr) {
            none = nullptr;
            if (entry->next.compare_exchange_weak(none, entry)) {
                do {
                    tail = tail_;
                    if (tail != entry)
                        break;
                    origin = entry;
                } while (!tail_.compare_exchange_weak(origin, &head_));
                break;
            } else {
                next = entry->next;
                if (next != nullptr) {
                    none = nullptr;
                    while (!head_.next.compare_exchange_weak(none, next))
                        none = nullptr;
                }
            }
        }

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
        AtomEntry() {}
        explicit AtomEntry(const ELEM& e) : elem(e) {}
        explicit AtomEntry(AtomEntry* n) : next(n) {}

        ELEM elem;
        std::atomic<AtomEntry*> next {nullptr};
    }; // struct AtomEntry

    AtomEntry head_;
    std::atomic<AtomEntry*> tail_ {&head_};
}; // class AtomQueue

} // namespace flame

#endif // !FLAME_COMMON_ATOM_QUEUE_H