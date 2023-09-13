#ifndef __THREADKIT_EVENT_H__
#define __THREADKIT_EVENT_H__

#include <mutex>
#include <condition_variable>

namespace threadkit {

class Event final {
public:
    Event(uint32_t max_count = 0) : m_max_count(max_count), m_counter(0) {}
    void ResetMaxCount(uint32_t max_count) {
        m_max_count = max_count;
    }
    void Finish() {
        m_mutex.lock();
        ++m_counter;
        if (m_counter >= m_max_count) {
            m_cond.notify_one();
        }
        m_mutex.unlock();
    }
    void Wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() -> bool {
            return (m_counter >= m_max_count);
        });
        m_counter = 0;
    }

private:
    uint32_t m_max_count;
    uint32_t m_counter;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

}

#endif
