#include "threadkit/mpsc_queue.h"
using namespace std;

namespace threadkit {

/*
  based on
    - https://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
    - https://github.com/grpc/grpc/blob/master/src/core/lib/gprpp/mpscq.h
*/

bool MPSCQueue::Push(Node* node) {
    node->mpsc_next.store(nullptr, std::memory_order_relaxed);

    /*
      +--------+
      | m_tail | -> nil
      +--------+

      +------+
      | node | -> nil
      +------+
    */

    auto prev = m_tail.exchange(node, std::memory_order_acq_rel);

    /*
      +------+
      | prev | -> nil
      +------+

       m_tail
         |
         v
      +------+
      | node | -> nil
      +------+
    */

    prev->mpsc_next.store(node, std::memory_order_release);

    /*
                   m_tail
                     |
                     v
      +------+    +------+
      | prev | -> | node | -> nil
      +------+    +------+
    */

    return (prev == &m_stub);
}

pair<MPSCQueue::Node*, bool> MPSCQueue::Pop() {
    auto head = m_head;
    auto next = head->mpsc_next.load(std::memory_order_acquire);

    if (head == &m_stub) {
        if (!next) {
            /*
                 head       next
                  |          |
                  v          v
              +--------+
              | m_stub | -> nil
              +--------+
            */

            return {nullptr, true};
        }

        /*
             head
              |
              v
          +--------+    +------+
          | m_stub | -> | next | -> ...
          +--------+    +------+
        */

        m_head = next;

        /*
             head        m_head
              |            |
              v            v
          +--------+    +------+
          | m_stub | -> | next | -> ...
          +--------+    +------+
        */

        head = next;

        /*
                          head
                         m_head
                           |
                           v
          +--------+    +------+
          | m_stub | -> | next | -> ...
          +--------+    +------+
        */

        next = next->mpsc_next.load(std::memory_order_acquire);

        /*
                          head
                         m_head
                           |
                           v
          +--------+    +------+    +------+
          | m_stub | -> |      | -> | next |
          +--------+    +------+    +------+
        */
    }

    /*
              m_head
                |
                v
             +------+    +------+
      ... -> | head | -> | next |
             +------+    +------+
    */

    if (next) {
        /*
                  m_head
                    |
                    v
                 +------+    +------+
          ... -> | head | -> | next | -> ...
                 +------+    +------+
        */

        m_head = next;

        /*
                              m_head
                                |
                                v
                 +------+    +------+
          ... -> | head | -> | next | -> ...
                 +------+    +------+
        */

        return {head, false};
    }

    /*
              m_head     next
                |         |
                v         v
             +------+
      ... -> | head | -> nil
             +------+
    */

    auto tail = m_tail.load(std::memory_order_acquire);

    if (head != tail) {
        /*
                  m_head     next
                    |         |
                    v         v
                 +------+
          ... -> | head | -> nil
                 +------+

                 +------+
                 | node | -> nil
                 +------+
                    ^
                    |
                tail/m_tail
        */

        return {nullptr, false}; // inserting
    }

    /*
              m_head     next
                |         |
                v         v
             +------+
      ... -> | head | -> nil
             +------+
                ^
                |
            tail/m_tail
    */

    Push(&m_stub);

    /*
              m_head
                |
                v
             +------+           +--------+
      ... -> | head | -> ... -> | m_stub | -> nil
             +------+           +--------+
                ^                   ^
                |                   |
               tail               m_tail
    */

    next = head->mpsc_next.load(std::memory_order_acquire);

    /*
              m_head
                |
                v
             +------+    +------+
      ... -> | head | -> | next |
             +------+    +------+
                ^
                |
               tail
    */

    if (next) {
        /*
                  m_head
                    |
                    v
                 +------+    +------+
          ... -> | head | -> | next | -> ...
                 +------+    +------+
                    ^
                    |
                   tail
        */

        m_head = next;

        /*
                              m_head
                                |
                                v
                 +------+    +------+
          ... -> | head | -> | next | -> ...
                 +------+    +------+
                    ^
                    |
                   tail
        */

        return {head, false};
    }

    /*
              m_head     next
                |         |
                v         v
             +------+
      ... -> | head | -> nil
             +------+
                ^
                |
               tail

             +------+    +--------+
             | node | -> | m_stub | -> nil
             +------+    +--------+
                             ^
                             |
                           m_tail
    */

    return {nullptr, false}; // inserting
}

}
