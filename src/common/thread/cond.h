#ifndef FLAME_COMMON_THREAD_COND_H
#define FLAME_COMMON_THREAD_COND_H

#include "mutex.h"
#include "util/utime.h"

#include <cassert>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

namespace flame {

class Cond {
private:
    pthread_cond_t m_cond;
    Mutex *waiter_mutex;

    // don't allow copying
    void operator = (Cond &c);
    Cond(const Cond &c);

public:
    Cond() : waiter_mutex(NULL) {
        int r = pthread_cond_init(&m_cond, NULL);
        assert(r == 0);
    }

    explicit Cond(Mutex &mutex) : waiter_mutex(&mutex) {
        assert(waiter_mutex != NULL);
        int r = pthread_cond_init(&m_cond, NULL);
        assert(r == 0);
    }

    virtual ~Cond() {
        pthread_cond_destroy(&m_cond);
    }

    void set_mutex(Mutex &mutex) {
        assert(waiter_mutex == NULL);
        waiter_mutex = &mutex;
    }

    int wait() {
        assert(waiter_mutex != NULL && waiter_mutex->is_locked());

        waiter_mutex->_pre_unlock();
        int r = pthread_cond_wait(&m_cond, &waiter_mutex->m_mutex);
        waiter_mutex->_post_lock();
        return r;
    }

    int wait_until(utime_t when) {
        assert(waiter_mutex != NULL && waiter_mutex->is_locked());

        struct timespec ts;
        when.to_timespec(&ts);

        waiter_mutex->_pre_unlock();
        int r = pthread_cond_timedwait(&m_cond, &waiter_mutex->m_mutex, &ts);
        waiter_mutex->_post_lock();

        return r;
    }

    int wait_interval(utime_t interval) {
        utime_t when = utime_t::now();
        when += interval;
        return wait_until(when);
    }

    int broadcast() {
        assert(waiter_mutex != NULL);
        int r = pthread_cond_broadcast(&m_cond);
        return r;
    }

    int signal() {
        assert(waiter_mutex != NULL);
        int r = pthread_cond_signal(&m_cond);
        return r;
    }
}; // class Cond

} // namespace flame

#endif // FLAME_COMMON_THREAD_COND_H