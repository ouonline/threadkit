#ifndef __THREADKIT_MCS_SPINLOCK_H__
#define __THREADKIT_MCS_SPINLOCK_H__

#include "common.h"
#include <atomic>

namespace threadkit {

class MCSSpinLock final {
public:
    struct alignas(CACHELINE_SIZE) Node final {
        std::atomic<Node*> next;
        std::atomic<uint32_t> locked;
    };

public:
    void Lock(Node*);
    void Unlock(Node*);

private:
    std::atomic<Node*> m_tail = {nullptr};
};

}

#endif
