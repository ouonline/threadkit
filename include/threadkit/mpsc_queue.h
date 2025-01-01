#ifndef __THREADKIT_MPSC_QUEUE_H__
#define __THREADKIT_MPSC_QUEUE_H__

#include <atomic>
#include <utility> // std::pair
#include "common.h"

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
       @return `first` is the popped node and `second` indicates whether the queue is empty or not.
       `first` may be nullptr if insertion is happening, and `second` is false at that moment.
    */
    std::pair<Node*, bool> Pop();

    /** @brief try to pop a node until no insertion is happening. */
    Node* PopNode() {
        std::pair<Node*, bool> ret_pair;
        do {
            ret_pair = Pop();
        } while (!ret_pair.first && !ret_pair.second);
        return ret_pair.first;
    }

private:
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
