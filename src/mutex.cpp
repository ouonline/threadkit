#include "threadkit/mutex.h"
#include "futex_wrapper.h"

/** based on http://locklessinc.com/articles/mutex_cv_futex */

namespace threadkit {

#define STATE_LOCKED ((uint32_t)1)
#define STATE_CONTENDED ((uint32_t)0x100)

#define STATE_LOCKED_AND_CONTENDED (STATE_LOCKED | STATE_CONTENDED)
#define STATE_LOCKED_AND_UNCONTENDED STATE_LOCKED
#define STATE_UNLOCKED_AND_UNCONTENDED ((uint32_t)0)

bool Mutex::TryLock() {
    auto prev = m_state.fetch_or(STATE_LOCKED, std::memory_order_acq_rel);
    return ((prev & STATE_LOCKED) == 0);
}

void Mutex::Lock() {
    if (TryLock()) {
        return;
    }

    // have to sleep
    auto prev = m_state.exchange(STATE_LOCKED_AND_CONTENDED, std::memory_order_relaxed);
    while (prev & STATE_LOCKED) {
        FutexWait(reinterpret_cast<uint32_t*>(&m_state), STATE_LOCKED_AND_CONTENDED);
        prev = m_state.exchange(STATE_LOCKED_AND_CONTENDED, std::memory_order_release);
    }
}

void Mutex::Unlock() {
    auto expected = STATE_LOCKED_AND_UNCONTENDED;
    if (m_state.compare_exchange_strong(expected, STATE_UNLOCKED_AND_UNCONTENDED,
                                        std::memory_order_seq_cst, std::memory_order_relaxed)) {
        return;
    }

    // unlock and wake someone up
    m_state.store(STATE_UNLOCKED_AND_UNCONTENDED, std::memory_order_seq_cst);
    FutexWakeOne(reinterpret_cast<uint32_t*>(&m_state));
}

}
