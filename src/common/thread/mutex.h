#ifndef FLAME_COMMON_THREAD_MUTEX_H
#define FLAME_COMMON_THREAD_MUTEX_H

#include <cassert>
#include <string>
#include <pthread.h>

namespace flame {

enum MutexType {
    MUTEX_TYPE_DEFAULT = 0,
    MUTEX_TYPE_NORMAL,
    MUTEX_TYPE_ERRORCHECK,
    MUTEX_TYPE_RECURSIVE
};

class Mutex {
private:
    MutexType m_type;
    pthread_mutex_t m_mutex;
    int m_nlock;
    pthread_t m_owner;

    // don't allow copying
    void operator = (const Mutex& M);
    Mutex(const Mutex& M);

public:
    Mutex(MutexType t = MUTEX_TYPE_DEFAULT);
    ~Mutex();
    
    bool is_locked() const { return (m_nlock > 0); }
    bool is_owned() const { return m_nlock > 0 && m_owner == pthread_self(); }
    
    bool trylock() {
        int r = pthread_mutex_trylock(&m_mutex);
        if (r == 0) 
            _post_lock();
        return r == 0;
    }

    void lock() {
        int r = pthread_mutex_lock(&m_mutex);
        assert(r == 0);
        _post_lock();
    }

    void unlock() {
        _pre_unlock();
        int r = pthread_mutex_unlock(&m_mutex);
        assert(r == 0);
    }

    void _post_lock() {
        if (m_type != MUTEX_TYPE_RECURSIVE) {
            assert(m_nlock == 0);
            m_owner = pthread_self();
        }
        m_nlock++;
    }

    void _pre_unlock() {
        assert(m_nlock > 0);
        --m_nlock;
        if (m_type != MUTEX_TYPE_RECURSIVE) {
            assert(m_owner == pthread_self());
            m_owner = 0;
            assert(m_nlock == 0);
        }
    }

    friend class Cond;
}; // class Mutex

class MutexLocker {
private:
    Mutex &mutex;

public:
    explicit MutexLocker(Mutex& m) : mutex(m) {
        mutex.lock();
    }

    ~MutexLocker() {
        mutex.unlock();
    }
}; // class MutexLocker

} // namespace flame

#endif // FLAME_COMMON_THREAD_MUTEX_H