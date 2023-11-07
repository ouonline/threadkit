#ifndef __THREADKIT_BARRIER_H__
#define __THREADKIT_BARRIER_H__

#include "event_count.h"
#include <atomic>

namespace threadkit {

class Barrier final {
public:
    Barrier(uint32_t max_count = 0) : m_max_count(max_count), m_counter(0) {}
    void ResetMaxCount(uint32_t max_count) {
        m_max_count = max_count;
    }
    void Wait() {
        auto key = m_cond.PrepareWait();
        auto counter = m_counter.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (counter < m_max_count) {
            m_cond.CommitWait(key);
        } else {
            m_counter.store(0, std::memory_order_release);
            m_cond.CancelWait();
            m_cond.NotifyAll();
        }
    }

private:
    uint32_t m_max_count;
    std::atomic<uint32_t> m_counter;
    EventCount m_cond;
};

}

#endif
