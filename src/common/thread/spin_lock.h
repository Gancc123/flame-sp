#ifndef FLAME_COMMON_THREAD_SPIN_LOCK_H
#define FLAME_COMMON_THREAD_SPIN_LOCK_H

#include <pthread.h>

namespace flame {

enum SpinLockType {
    SPIN_LOCK_TYPE_DEFAULT = 0,
    SPIN_LOCK_TYPE_PROCESS_PRIVATE,
    SPIN_LOCK_TYPE_PROCESS_SHARED
};


class SpinLock {
public:
    explicit SpinLock(SpinLockType t = SPIN_LOCK_TYPE_DEFAULT) {
        int pshared = PTHREAD_PROCESS_PRIVATE;
        if(t == SPIN_LOCK_TYPE_PROCESS_SHARED){
            pshared = PTHREAD_PROCESS_SHARED;
        }
        pthread_spin_init(&m_lock, pshared);
    }

    ~SpinLock() {
        pthread_spin_destroy(&m_lock);
    }

    int lock() { return pthread_spin_lock(&m_lock); }

    int try_lock() { return pthread_spin_trylock(&m_lock); }

    int unlock() { return pthread_spin_unlock(&m_lock); }

private:
    pthread_spinlock_t m_lock;
}; // class SpinLocker

class SpinLocker {
private:
    SpinLock &spinlock;

public:
    explicit SpinLocker(SpinLock &l) : spinlock(l) {
        spinlock.lock();
    }

    ~SpinLocker() {
        spinlock.unlock();
    }
}; // class SpinLocker


} // namespace flame

#endif