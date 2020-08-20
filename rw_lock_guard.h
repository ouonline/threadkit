#ifndef __THREADKIT_RW_LOCK_GUARD_H__
#define __THREADKIT_RW_LOCK_GUARD_H__

#include <pthread.h>

namespace outils {

class RWLockGuard final {
public:
    enum {
        RDLOCK = 0,
        WRLOCK = 1,
    };

public:
    RWLockGuard(pthread_rwlock_t* lock, int lock_type) : m_lock(lock) {
        if (lock_type == 0) {
            pthread_rwlock_rdlock(lock);
        } else {
            pthread_rwlock_wrlock(lock);
        }
    }
    ~RWLockGuard() {
        pthread_rwlock_unlock(m_lock);
    }

private:
    pthread_rwlock_t* m_lock;

private:
    RWLockGuard(const RWLockGuard&);
    RWLockGuard& operator=(const RWLockGuard&);
};

}

#endif
