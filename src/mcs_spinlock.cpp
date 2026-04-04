#include "threadkit/mcs_spinlock.h"
using namespace std;

namespace threadkit {

void MCSSpinLock::Lock(Node* node) {
    node->next.store(nullptr, memory_order_relaxed);

    /*
      +------+
      | node | -> nil
      +------+
    */

    Node* prev = m_tail.exchange(node, memory_order_acq_rel);

    /*
      +-------+
      | prev? |
      +-------+

      m_tail
         |
         v
      +------+
      | node | -> nil
      +------+
    */

    if (prev) {
        /*
          +------+
          | prev | -> nil
          +------+
        */

        node->locked.store(1, memory_order_relaxed);
        prev->next.store(node, memory_order_release);

        /*
                      m_tail
                         |
                         v
          +------+    +------+
          | prev | -> | node | -> nil
          +------+    +------+
        */

        while (node->locked.load(memory_order_acquire)) {
            cpu_relax();
        }
    }
}

void MCSSpinLock::Unlock(Node* node) {
    MCSSpinLock::Node* next = node->next.load(memory_order_relaxed);
    if (!next) {
        /*
          +------+
          | node | -> nil
          +------+
        */

        MCSSpinLock::Node* head = node;
        if (m_tail.compare_exchange_strong(head, nullptr, memory_order_acq_rel,
                                           memory_order_relaxed)) {
            /*
              +------+
              | node | -> nil
              +------+

              m_tail
                 |
                 v
                nil
            */
            return;
        }

        /*
          +------+
          | node | -> nil
          +------+

                 m_tail
                    |
                    v
                 +-----+
          ... -> | ... | -> nil
                 +-----+
        */

        while ((next = node->next.load(memory_order_acquire)) == nullptr) {
            cpu_relax();
        }

        /*
                             m_tail
                                |
                                v
          +------+           +-----+
          | node | -> ... -> | ... | -> nil
          +------+           +-----+
        */
    }

    next->locked.store(0, memory_order_release);
}

}
