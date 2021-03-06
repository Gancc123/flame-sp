#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <signal.h>

#include "thread.h"
#include "io_priority.h"
#include "signal.h"

#include "util/clog.h"

#include <sched.h>

namespace flame {

static int _set_affinity(int id) {
    if (id >= 0 && id < CPU_SETSIZE) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        CPU_SET(id, &cpuset);

        if (sched_setaffinity(0, sizeof(cpuset), &cpuset) < 0)
            return -errno;
        /* guaranteed to take effect immediately */
        sched_yield();
    }
    return 0;
}

static inline long _get_page_mask() {
    return ~(sysconf(_SC_PAGESIZE) - 1);
}

Thread::Thread()
    : thread_id(0),
      pid(0),
      ioprio_class(-1),
      ioprio_priority(-1),
      cpuid(-1),
      thread_name(NULL)
{
    // do nothing
}

Thread::~Thread() {
    // do nothing
}

void* Thread::_entry_func(void* arg) {
    ((Thread *) arg)->entry_wrapper();
    return nullptr;
}

void Thread::entry_wrapper() {
    int p = flame_gettid(); // may return -ENOSYS on other platforms
    if (p > 0)
        pid = p;
    if (pid &&
        ioprio_class >= 0 &&
        ioprio_priority >= 0) {
        flame_ioprio_set(IOPRIO_WHO_PROCESS,
                        pid,
                        IOPRIO_PRIO_VALUE(ioprio_class, ioprio_priority));
    }
    if (pid && cpuid >= 0)
        _set_affinity(cpuid);

    pthread_setname_np(pthread_self(), thread_name);
    entry();
}

const pthread_t& Thread::get_thread_id() const {
    return thread_id;
}

bool Thread::is_started() const {
    return thread_id != 0;
}

bool Thread::am_self() const {
    return pthread_equal(pthread_self(), thread_id);
}

int Thread::kill(int signal) {
    if (thread_id)
        return pthread_kill(thread_id, signal);
    else
        return -EINVAL;
}

int Thread::try_create(size_t stacksize) {
    pthread_attr_t* thread_attr = NULL;
    pthread_attr_t thread_attr_loc;

    if (pid != 0 || thread_id != 0) {
        // thread has been created.
        return 0;
    }

    stacksize &= _get_page_mask();  // must be multiple of page
    if (stacksize) {
        thread_attr = &thread_attr_loc;
        pthread_attr_init(thread_attr);
        pthread_attr_setstacksize(thread_attr, stacksize);
    }

    int r;

    // The child thread will inherit our signal mask.  Set our signal mask to
    // the set of signals we want to block.  (It's ok to block signals more
    // signals than usual for a little while-- they will just be delivered to
    // another thread or delieverd to this thread later.)
    sigset_t old_sigset;
    // if (g_code_env == CODE_ENVIRONMENT_LIBRARY) {
    //     block_signals(NULL, &old_sigset);
    // } else {
    //     int to_block[] = {SIGPIPE, 0};
    //     block_signals(to_block, &old_sigset);
    // }
    int to_block[] = {SIGPIPE, 0};
    block_signals(to_block, &old_sigset);
    r = pthread_create(&thread_id, thread_attr, _entry_func, (void *) this);
    restore_sigset(&old_sigset);

    if (thread_attr) {
        pthread_attr_destroy(thread_attr);
    }

    return r;
}

void Thread::create(const char *name, size_t stacksize) {
    if (pid != 0 || thread_id != 0) {
        // thread has been created.
        return;
    }

    assert(strlen(name) < 16);
    thread_name = name;

    int ret = try_create(stacksize);
    if (ret != 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Thread::try_create(): pthread_create "
                                   "failed with error %d", ret);
        dout_emergency(buf);
        assert(ret == 0);
    }
}

int Thread::join(void **prval) {
    if (thread_id == 0) {
        assert("join on thread that was never started" == 0);
        return -EINVAL;
    }

    int status = pthread_join(thread_id, prval);
    if (status != 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Thread::join(): pthread_join "
                                   "failed with error %d\n", status);
        dout_emergency(buf);
        assert(status == 0);
    }

    thread_id = 0;
    return status;
}

int Thread::detach() {
    if (thread_id == 0) {
        assert("join on thread that was never started" == 0);
        return -EINVAL;
    }
    return pthread_detach(thread_id);
}

int Thread::set_ioprio(int cls, int prio) {
    // fixme, maybe: this can race with create()
    ioprio_class = cls;
    ioprio_priority = prio;
    if (pid && cls >= 0 && prio >= 0)
        return flame_ioprio_set(IOPRIO_WHO_PROCESS,
                               pid,
                               IOPRIO_PRIO_VALUE(cls, prio));
    return 0;
}

int Thread::set_affinity(int id) {
    int r = 0;
    cpuid = id;
    if (pid && flame_gettid() == pid)
        r = _set_affinity(id);
    return r;
}

} // namespace flame
