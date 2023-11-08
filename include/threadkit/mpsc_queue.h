#ifndef __THREADKIT_MPSC_QUEUE_H__
#define __THREADKIT_MPSC_QUEUE_H__

#include <atomic>

namespace threadkit {

/** a lock-free queue implementation for multi-producer-single-consumer */

class MPSCQueue final {
public:
    struct Node {
        std::atomic<Node*> mpsc_next;
    };

public:
    MPSCQueue() : m_head(&m_stub), m_tail(&m_stub) {
        m_stub.mpsc_next.store(nullptr, std::memory_order_relaxed);
    }

    /** @return tells whether queue may be empty or not before inserting `node`. */
    bool Push(Node* node);

    /**
       @param [out] is_empty indicates whether the queue is empty or not.
       @return may be nullptr if insertion is happening and `is_empty` is false.
    */
    Node* Pop(bool* is_empty);

private:
    // l1 cache line size for most CPUs
    static constexpr int CACHELINE_SIZE = 64;

    union {
        Node* m_head;
        char padding[CACHELINE_SIZE];
    };
    std::atomic<Node*> m_tail;
    Node m_stub;

private:
    MPSCQueue(const MPSCQueue&) = delete;
    MPSCQueue(MPSCQueue&&) = delete;
    void operator=(const MPSCQueue&) = delete;
    void operator=(MPSCQueue&&) = delete;
};

}

#endif
