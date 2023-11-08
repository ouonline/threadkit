#include "threadkit/event_count.h"
#include "futex_wrapper.h"
using namespace std;

// based on https://github.com/facebook/folly/blob/main/folly/experimental/EventCount.h

namespace threadkit {

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline uint32_t* GetEpochAddr(uint64_t* v) {
    return reinterpret_cast<uint32_t*>(v) + 1;
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static inline uint32_t* GetEpochAddr(uint64_t* v) {
    return reinterpret_cast<uint32_t*>(v);
}
#endif

#define EPOCH_SHIFT 32
#define ONE_WAITER 1
#define ONE_EPOCH ((uint64_t)1 << EPOCH_SHIFT)
#define WAITER_MASK ((uint64_t)0xffffffff)

EventCount::Key EventCount::PrepareWait() {
    uint64_t prev = m_val.fetch_add(ONE_WAITER, std::memory_order_acq_rel);
    return (prev >> EPOCH_SHIFT);
}

void EventCount::CancelWait() {
    /*
      the faster #waiters gets to 0, the less likely it is that we'll do spurious wakeups
      (and thus system calls).
    */
    m_val.fetch_sub(ONE_WAITER, std::memory_order_seq_cst);
}

void EventCount::CommitWait(EventCount::Key v) {
    volatile uint32_t* epoch = GetEpochAddr(reinterpret_cast<uint64_t*>(&m_val));
    while (*epoch == v) {
        FutexWait(const_cast<uint32_t*>(epoch), v);
    }
    /*
      the faster #waiters gets to 0, the less likely it is that we'll do spurious wakeups
      (and thus system calls).
    */
    m_val.fetch_sub(ONE_WAITER, std::memory_order_seq_cst);
}

void EventCount::NotifyOne() {
    auto prev = m_val.fetch_add(ONE_EPOCH, std::memory_order_acq_rel);
    if (prev & WAITER_MASK) {
        FutexWakeOne(GetEpochAddr(reinterpret_cast<uint64_t*>(&m_val)));
    }
}

void EventCount::NotifyAll() {
    auto prev = m_val.fetch_add(ONE_EPOCH, std::memory_order_acq_rel);
    if (prev & WAITER_MASK) {
        FutexWakeAll(GetEpochAddr(reinterpret_cast<uint64_t*>(&m_val)));
    }
}

}
