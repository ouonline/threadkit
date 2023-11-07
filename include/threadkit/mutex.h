#ifndef __THREADKIT_MUTEX_H__
#define __THREADKIT_MUTEX_H__

#include <stdint.h>
#include <atomic>

namespace threadkit {

class Mutex final {
public:
    Mutex() : m_state(0) {
        static_assert(sizeof(m_state) == sizeof(uint32_t), "atomic size mismatch");
    }
    void Lock();
    void Unlock();

private:
    std::atomic<uint32_t> m_state;

private:
    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    void operator=(const Mutex&) = delete;
    void operator=(Mutex&&) = delete;
};

}

#endif
