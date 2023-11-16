#ifndef __THREADKIT_EVENT_COUNT_H__
#define __THREADKIT_EVENT_COUNT_H__

#include <stdint.h>
#include <atomic>

namespace threadkit {

class EventCount final {
public:
    typedef uint32_t Key;

public:
    EventCount() : m_val(0) {
        static_assert(sizeof(m_val) == sizeof(uint64_t), "atomic size mismatch");
    }

    Key PrepareWait();
    void CancelWait();
    void CommitWait(Key);
    void NotifyOne();
    void NotifyAll();

    template <typename Predicate>
    void Wait(Predicate&& stop_waiting) {
        if (stop_waiting()) {
            return;
        }

        while (true) {
            auto key = PrepareWait();
            if (stop_waiting()) {
                CancelWait();
                return;
            }
            CommitWait(key);
        }
    }

private:
    // the epoch in the most significant 32 bits and the waiter count in the least significant 32 bits
    std::atomic<uint64_t> m_val;

private:
    EventCount(const EventCount&) = delete;
    EventCount(EventCount&&) = delete;
    void operator=(const EventCount&) = delete;
    void operator=(EventCount&&) = delete;
};

}

#endif
