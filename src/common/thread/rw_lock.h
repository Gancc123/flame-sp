#ifndef FLAME_COMMON_THREAD_RWLOCK_H
#define FLAME_COMMON_THREAD_RWLOCK_H

#include <pthread.h>
#include <cassert>

namespace flame {

class RWLock {
public:
    RWLock() {
        int r = pthread_rwlock_init(&rwlock_, nullptr);
        assert(!r);
    }

    ~RWLock() { pthread_rwlock_destroy(&rwlock_); }

    void wrlock() { pthread_rwlock_wrlock(&rwlock_); }
    bool try_wrlock() { return pthread_rwlock_trywrlock(&rwlock_) == 0; }

    void rdlock() { pthread_rwlock_rdlock(&rwlock_); }
    bool try_rdlock() { return pthread_rwlock_tryrdlock(&rwlock_) == 0; }

    void unlock() { pthread_rwlock_unlock(&rwlock_); }

    RWLock(const RWLock&) = delete;
    RWLock(RWLock&&) = delete;
    RWLock& operator = (const RWLock&) = delete;
    RWLock& operator = (RWLock&&) = delete;

private:
    pthread_rwlock_t rwlock_;
};

class ReadLocker {
public:
    explicit ReadLocker(RWLock& lock) : rwlock_(lock) {
        rwlock_.rdlock();
    }

    ~ReadLocker() {
        rwlock_.unlock();
    }

private:
    RWLock& rwlock_;
}; // class RWRdLocker

class WriteLocker {
public:
    explicit WriteLocker(RWLock& lock) : rwlock_(lock) {
        rwlock_.wrlock();
    }

    ~WriteLocker() {
        rwlock_.unlock();
    }

private:
    RWLock& rwlock_;
}; // class RWWrLocker

}

#endif //FLAME_COMMON_THREAD_RWLOCK_H