#ifndef __THREADKIT_MUTEX_LOCK_GUARD_H__
#define __THREADKIT_MUTEX_LOCK_GUARD_H__

#include <pthread.h>

namespace outils {

class MutexLockGuard final {
public:
    MutexLockGuard(pthread_mutex_t* lock) : m_lock(lock) {
        pthread_mutex_lock(lock);
    }
    ~MutexLockGuard() {
        pthread_mutex_unlock(m_lock);
    }

private:
    pthread_mutex_t* m_lock;

private:
    MutexLockGuard(const MutexLockGuard&);
    MutexLockGuard& operator=(const MutexLockGuard&);
};

}

#endif
