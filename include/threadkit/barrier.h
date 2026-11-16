#ifndef __THREADKIT_BARRIER_H__
#define __THREADKIT_BARRIER_H__

#include "cond_var.h"
#include <atomic>

namespace threadkit {

class Barrier final {
public:
    Barrier(uint32_t max_count = 0)
        : m_max_count(max_count), m_generation(0), m_counter(0) {}
    void ResetMaxCount(uint32_t max_count) {
        m_max_count = max_count;
    }
    void Wait() {
        if (m_max_count == 0) {
            return;
        }

        uint32_t gen = m_generation.load(std::memory_order_acquire);
        uint32_t count = m_counter.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (count == m_max_count) {
            m_counter.store(0, std::memory_order_release);
            m_generation.fetch_add(1, std::memory_order_acq_rel);
            m_cond.NotifyAll();
        } else {
            m_cond.Wait([&]() -> bool {
                return (m_generation.load(std::memory_order_acquire) != gen);
            });
        }
    }

private:
    uint32_t m_max_count;
    std::atomic<uint32_t> m_generation;
    std::atomic<uint32_t> m_counter;
    CondVar m_cond;
};

}

#endif
