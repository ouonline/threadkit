#ifndef __THREADKIT_SPSC_BOUNDED_QUEUE_H__
#define __THREADKIT_SPSC_BOUNDED_QUEUE_H__

/** single-producer-single-consumer bounded queue implementation */

#include <atomic>
#include <vector>
#include "common.h"

namespace threadkit {

template <typename T>
class SPSCBoundedQueue final {
public:
    SPSCBoundedQueue(uint32_t max_nr_item) : m_buffer(max_nr_item + 1) {
        m_head.store(0, std::memory_order_relaxed);
        m_tail.store(0, std::memory_order_relaxed);
    }

    SPSCBoundedQueue(SPSCBoundedQueue&& rhs) {
        DoMove(std::move(rhs));
    }

    void operator=(SPSCBoundedQueue&& rhs) {
        DoMove(std::move(rhs));
    }

    template <typename DataType>
    bool Push(DataType&& data) {
        uint32_t tail = m_tail.load(std::memory_order_relaxed);
        uint32_t next_tail = Next(tail);
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false; // full
        }

        m_buffer[tail] = std::forward<DataType>(data);
        m_tail.store(next_tail, std::memory_order_release);
        return true;
    }

    template <typename DataType = T>
    bool Pop(DataType* data = nullptr) {
        uint32_t head = m_head.load(std::memory_order_relaxed);
        if (head == m_tail.load(std::memory_order_acquire)) {
            return false; // empty
        }

        if (data) {
            *data = std::move(m_buffer[head]);
        } else {
            m_buffer[head].~T();
        }

        m_head.store(Next(head), std::memory_order_release);
        return true;
    }

    bool IsEmpty() const {
        return (m_head.load(std::memory_order_relaxed) == m_tail.load(std::memory_order_relaxed));
    }

    bool IsFull() const {
        return (Next(m_tail.load(std::memory_order_relaxed)) == m_head.load(std::memory_order_relaxed));
    }

private:
    uint32_t Next(uint32_t cursor) const {
        return (cursor + 1) % m_buffer.size();
    }

    void DoMove(SPSCBoundedQueue&& rhs) {
        m_head.store(rhs.m_head.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_tail.store(rhs.m_tail.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_buffer = std::move(rhs.m_buffer);

        rhs.m_head.store(0, std::memory_order_relaxed);
        rhs.m_tail.store(0, std::memory_order_relaxed);
    }

private:
    union {
        std::atomic<uint32_t> m_head;
        char padding1[CACHELINE_SIZE];
    };
    union {
        std::atomic<uint32_t> m_tail;
        char padding2[CACHELINE_SIZE];
    };
    std::vector<T> m_buffer;

private:
    SPSCBoundedQueue(const SPSCBoundedQueue&) = delete;
    void operator=(const SPSCBoundedQueue&) = delete;
};

}

#endif
