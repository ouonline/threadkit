#ifndef __THREADKIT_MUTEX_H__
#define __THREADKIT_MUTEX_H__

#include <stdint.h>

namespace threadkit {

class Mutex final {
public:
    Mutex() : m_state(0) {}
    void Lock();
    bool TryLock();
    void Unlock();

private:
    uint32_t m_state;

private:
    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    void operator=(const Mutex&) = delete;
    void operator=(Mutex&&) = delete;
};

}

#endif
