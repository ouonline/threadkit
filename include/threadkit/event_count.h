#ifndef __THREADKIT_EVENT_COUNT_H__
#define __THREADKIT_EVENT_COUNT_H__

#include <stdint.h>

namespace threadkit {

class EventCount final {
public:
    typedef uint32_t Key;

public:
    EventCount() : m_val(0) {}
    Key PrepareWait();
    void CancelWait();
    void CommitWait(Key);
    void NotifyOne();
    void NotifyAll();

    template <typename Predicate>
    void Wait(Predicate&& predicate) {
        if (predicate()) {
            return;
        }

        while (true) {
            auto key = PrepareWait();
            if (predicate()) {
                CancelWait();
                return;
            }
            CommitWait(key);
        }
    }

private:
    // the epoch in the most significant 32 bits and the waiter count in the least significant 32 bits
    // std::atomic's implementation is platform-dependent and doesn't fit for futex().
    uint64_t m_val;

private:
    EventCount(const EventCount&) = delete;
    EventCount(EventCount&&) = delete;
    void operator=(EventCount&&) = delete;
    void operator=(const EventCount&) = delete;
};

}

#endif
