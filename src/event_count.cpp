#include "threadkit/event_count.h"
#include <climits>
using namespace std;

// inspired by https://github.com/facebook/folly/blob/main/folly/experimental/EventCount.h

#ifdef _MSC_VER

#include <windows.h>

static inline void FutexWait(volatile uint32_t* addr, uint32_t* value_addr) {
    WaitOnAddress(addr, value_addr, sizeof(uint32_t), INFINITE);
}

static inline void FutexWakeOne(uint32_t* addr) {
    WakeByAddressSingle(addr);
}

static inline void FutexWakeAll(uint32_t* addr) {
    WakeByAddressAll(addr);
}

template <typename T1, typename T2>
static T1 AtomicFetchAndAdd(T1* v, T2 n) {
    return InterlockedExchangeAdd(v, n);
}

template <typename T1, typename T2>
static T1 AtomicFetchAndSub(T1* v, T2 n) {
    return InterlockedExchangeSubtract(v, n);
}

#else

#include <linux/futex.h>
#include <sys/syscall.h> // SYS_futex
#include <unistd.h> // syscall()

// refer to http://locklessinc.com/articles/futex_cheat_sheet/

static inline void FutexWait(volatile uint32_t* addr, const uint32_t* value_addr) {
	syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, *value_addr, nullptr, nullptr, 0);
}

static inline void FutexWakeOne(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

static inline void FutexWakeAll(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, INT_MAX, nullptr, nullptr, 0);
}

template <typename T1, typename T2>
static T1 AtomicFetchAndAdd(T1* v, T2 n) {
    return __sync_fetch_and_add(v, n);
}

template <typename T1, typename T2>
static T1 AtomicFetchAndSub(T1* v, T2 n) {
    return __sync_fetch_and_sub(v, n);
}

#endif

namespace threadkit {

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline uint32_t* GetEPochAddr(uint64_t* v) {
    return (uint32_t*)v + 1;
}
#else
static inline uint32_t* GetEPochAddr(uint64_t* v) {
    return (uint32_t*)v;
}
#endif

#define EPOCH_SHIFT 32
#define ONE_WAITER 1
#define ONE_EPOCH ((uint64_t)1 << EPOCH_SHIFT)
#define WAITER_MASK ((uint64_t)0xffffffff)

EventCount::Key EventCount::PrepareWait() {
    uint64_t prev = AtomicFetchAndAdd(&m_val, ONE_WAITER);
    return (prev >> EPOCH_SHIFT);
}

void EventCount::CancelWait() {
    AtomicFetchAndSub(&m_val, ONE_WAITER);
}

void EventCount::CommitWait(EventCount::Key v) {
    volatile uint32_t* epoch = GetEPochAddr(&m_val);
    while (*epoch == v) {
        FutexWait(epoch, &v);
    }
    AtomicFetchAndSub(&m_val, ONE_WAITER);
}

void EventCount::NotifyOne() {
    auto prev = AtomicFetchAndAdd(&m_val, ONE_EPOCH);
    if (prev & WAITER_MASK) { // TODO unlikely() in heavily load scenes
        FutexWakeOne(GetEPochAddr(&m_val));
    }
}

void EventCount::NotifyAll() {
    auto prev = AtomicFetchAndAdd(&m_val, ONE_EPOCH);
    if (prev & WAITER_MASK) { // TODO unlikely() in heavily load scenes
        FutexWakeAll(GetEPochAddr(&m_val));
    }
}

}
