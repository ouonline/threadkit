#ifndef __THREADKIT_EVENT_COUNT_H__
#define __THREADKIT_EVENT_COUNT_H__

#include "timeval.h"
#include <stdint.h>

namespace threadkit {

class EventCount final {
public:
    typedef uint32_t Key;

public:
    EventCount() : m_val(0) {}
    Key PrepareWait();
    void CancelWait();
    void CommitWait(Key, const TimeVal* timeout = nullptr);
    void NotifyOne();
    void NotifyAll();

    template <typename Predicate>
    void Wait(Predicate&& stop_waiting, const TimeVal* timeout = nullptr) {
        if (stop_waiting()) {
            return;
        }

        while (true) {
            auto key = PrepareWait();
            if (stop_waiting()) {
                CancelWait();
                return;
            }
            CommitWait(key, timeout);
        }
    }

private:
    // the epoch in the most significant 32 bits and the waiter count in the least significant 32 bits
    uint64_t m_val;

private:
    EventCount(const EventCount&) = delete;
    EventCount(EventCount&&) = delete;
    void operator=(const EventCount&) = delete;
    void operator=(EventCount&&) = delete;
};

}

#endif
