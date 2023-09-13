#ifndef __THREADKIT_BARRIER_H__
#define __THREADKIT_BARRIER_H__

#include <mutex>
#include <condition_variable>

namespace threadkit {

class Barrier final {
public:
    Barrier(uint32_t max_count = 0) : m_max_count(max_count), m_counter(0) {}
    void ResetMaxCount(uint32_t max_count) {
        m_max_count = max_count;
    }
    void Wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_counter < m_max_count) {
            ++m_counter;
            if (m_counter < m_max_count) {
                m_cond.wait(lock);
            } else {
                m_counter = 0;
                m_cond.notify_all();
            }
        }
    }

private:
    uint32_t m_max_count;
    uint32_t m_counter;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

}

#endif
