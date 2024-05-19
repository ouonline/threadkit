#ifndef __THREADKIT_SPSC_RING_BUFFER_H__
#define __THREADKIT_SPSC_RING_BUFFER_H__

/** single-producer-single-consumer ring buffer implementation */

#include <atomic>
#include <vector>

namespace threadkit {

template <typename T>
class SPSCRingBuffer final {
public:
    SPSCRingBuffer(uint32_t max_nr_item) : m_buffer(max_nr_item + 1) {
        m_head.store(0, std::memory_order_relaxed);
        m_tail.store(0, std::memory_order_relaxed);
    }

    SPSCRingBuffer(SPSCRingBuffer&& rhs) {
        DoMove(std::move(rhs));
    }

    void operator=(SPSCRingBuffer&& rhs) {
        DoMove(std::move(rhs));
    }

    template <typename ItemType>
    bool Push(ItemType&& item) {
        uint32_t tail = m_tail.load(std::memory_order_relaxed);
        uint32_t next_tail = Next(tail);
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false; // full
        }

        m_buffer[tail] = std::forward<ItemType>(item);
        m_tail.store(next_tail, std::memory_order_release);
        return true;
    }

    template <typename ItemType>
    bool Pop(ItemType* item) {
        uint32_t head = m_head.load(std::memory_order_relaxed);
        if (head == m_tail.load(std::memory_order_acquire)) {
            return false; // empty
        }

        *item = std::move(m_buffer[head]);
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

    void DoMove(SPSCRingBuffer&& rhs) {
        m_head.store(rhs.m_head.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_tail.store(rhs.m_tail.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_buffer = std::move(rhs.m_buffer);

        rhs.m_head.store(0, std::memory_order_relaxed);
        rhs.m_tail.store(0, std::memory_order_relaxed);
    }

private:
    // cache line size for most CPUs
    static constexpr int CACHELINE_SIZE = 64;

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
    SPSCRingBuffer(const SPSCRingBuffer&) = delete;
    void operator=(const SPSCRingBuffer&) = delete;
};

}

#endif
