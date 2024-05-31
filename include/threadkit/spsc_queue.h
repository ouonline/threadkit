#ifndef __THREADKIT_SPSC_QUEUE_H__
#define __THREADKIT_SPSC_QUEUE_H__

/**
   a lock-free queue implementation for single-producer-single-consumer based on
   https://www.1024cores.net/home/lock-free-algorithms/queues/unbounded-spsc-queue
*/

#include <atomic>

namespace threadkit {

template <typename T>
class SPSCQueue final {
private:
    struct Node final {
        Node() {}

        template <typename DataType>
        Node(DataType&& d) : data(std::forward<DataType>(d)) {}

        std::atomic<Node*> next;
        T data;
    };

private:
    template <typename DataType>
    Node* AllocNode(DataType&& data) {
        /*
          +---------+           +-------------+
          | m_first | -> ... -> | m_head_copy | -> ...
          +---------+           +-------------+
        */
        if (m_first != m_head_copy) {
            Node* node = m_first;
            m_first = m_first->next.load(std::memory_order_relaxed);
            node->data = std::forward<DataType>(data);
            return node;
        }

        m_head_copy = m_head.load(std::memory_order_consume);

        if (m_first != m_head_copy) {
            Node* node = m_first;
            m_first = m_first->next.load(std::memory_order_relaxed);
            node->data = std::forward<DataType>(data);
            return node;
        }

        return new Node(std::forward<DataType>(data));
    }

public:
    SPSCQueue() {
        Node* node = new Node();
        node->next.store(nullptr, std::memory_order_relaxed);
        m_first = m_head_copy = m_tail = node;
        m_head.store(node, std::memory_order_release);

        /*
          m_head/m_tail/m_first/m_head_copy
                       |
                       v
                    +------+
                    | node | -> nil
                    +------+
        */
    }

    ~SPSCQueue() {
        Node* node = m_first;
        do {
            Node* next = node->next.load(std::memory_order_relaxed);
            delete node;
            node = next;
        } while (node);
    }

    template <typename DataType>
    void Push(DataType&& data) {
        Node* node = AllocNode(std::forward<DataType>(data));
        node->next.store(nullptr, std::memory_order_relaxed);

        /*
          +------+
          | node | -> nil
          +------+
        */


        m_tail->next.store(node, std::memory_order_release);

        /*
                 +--------+    +------+
          ... -> | m_tail | -> | node | -> nil
                 +--------+    +------+
        */

        m_tail = node;

        /*               m_tail
                           |
                           v
          +--------+    +------+
          |        | -> | node | -> nil
          +--------+    +------+
        */
    }

    template <typename DataType = T>
    bool Pop(DataType* data = nullptr) {
        Node* head = m_head.load(std::memory_order_relaxed);
        Node* next = head->next.load(std::memory_order_acquire);

        /*
          +--------+    +------+
          | m_head | -> | next | -> ...
          +--------+    +------+
        */

        if (next) {
            if (data) {
                *data = std::move(next->data);
            } else {
                next->data.~T();
            }

            m_head.store(next, std::memory_order_release);

            /*               m_head
                               |
                               v
              +--------+    +------+
              |        | -> | next | -> ...
              +--------+    +------+
            */

            return true;
        }

        return false;
    }

private:
    // l1 cache line size for most CPUs
    static constexpr int CACHELINE_SIZE = 64;

    // consumer
    union {
        std::atomic<Node*> m_head;
        char padding1[CACHELINE_SIZE];
    };
    // producer
    Node* m_first;
    Node* m_head_copy;
    Node* m_tail;

private:
    SPSCQueue(const SPSCQueue&) = delete;
    void operator=(const SPSCQueue&) = delete;
    SPSCQueue(SPSCQueue&&) = delete;
    void operator=(SPSCQueue&&) = delete;
};

}

#endif
