#ifndef __THREADKIT_MPMC_BOUNDED_QUEUE_H__
#define __THREADKIT_MPMC_BOUNDED_QUEUE_H__

/**
   a lock-free bounded queue implementation for multi-producer-multi-consumer based on
   https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
*/

#include <atomic>
#include <vector>

namespace threadkit {

template <typename T>
class MPMCBoundedQueue final {
private:
    struct Cell final {
        Cell() {}

        template <typename DataType>
        Cell(DataType&& d) : data(std::forward<DataType>(d)) {}

        std::atomic<uint64_t> seq;
        T data;
    };

public:
    // `sz` MUST be power of 2
    MPMCBoundedQueue(uint64_t sz) : m_buffer_mask(sz - 1), m_buffer(sz) {
        for (uint64_t i = 0; i < sz; ++i) {
            m_buffer[i].seq.store(i, std::memory_order_relaxed);
        }
        m_push_idx.store(0, std::memory_order_relaxed);
        m_pop_idx.store(0, std::memory_order_relaxed);
    }

    template <typename DataType>
    bool Push(DataType&& data) {
        Cell* cell;
        uint64_t idx = m_push_idx.load(std::memory_order_relaxed);
        while (true) {
            cell = &m_buffer[idx & m_buffer_mask];

            uint64_t seq = cell->seq.load(std::memory_order_acquire);
            if (seq == idx) {
                if (m_push_idx.compare_exchange_weak(idx, idx + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (seq < idx) {
                return false;
            } else {
                idx = m_push_idx.load(std::memory_order_relaxed);
            }
        }

        cell->data = std::forward<DataType>(data);
        cell->seq.store(idx + 1, std::memory_order_release);

        return true;
    }

    template <typename DataType = T>
    bool Pop(DataType* data = nullptr) {
        Cell* cell;
        uint64_t idx = m_pop_idx.load(std::memory_order_relaxed);
        while (true) {
            cell = &m_buffer[idx & m_buffer_mask];

            uint64_t seq = cell->seq.load(std::memory_order_acquire);
            if (seq == idx + 1) {
                if (m_pop_idx.compare_exchange_weak(idx, idx + 1, std::memory_order_relaxed)) {
                    break;
                }
            } else if (seq < idx + 1) {
                return false;
            } else {
                idx = m_pop_idx.load(std::memory_order_relaxed);
            }
        }

        if (data) {
            *data = std::move(cell->data);
        } else {
            cell->data.~T();
        }

        cell->seq.store(idx + m_buffer_mask + 1, std::memory_order_release);

        return true;
    }

private:
    // l1 cache line size for most CPUs
    static constexpr int CACHELINE_SIZE = 64;

    union {
        std::atomic<uint64_t> m_push_idx;
        char padding1[CACHELINE_SIZE];
    };
    union {
        std::atomic<uint64_t> m_pop_idx;
        char padding2[CACHELINE_SIZE];
    };
    const uint64_t m_buffer_mask;
    std::vector<Cell> m_buffer;

private:
    MPMCBoundedQueue(const MPMCBoundedQueue&) = delete;
    void operator=(const MPMCBoundedQueue&) = delete;
    MPMCBoundedQueue(MPMCBoundedQueue&&) = delete;
    void operator=(MPMCBoundedQueue&&) = delete;
};

}

#endif
