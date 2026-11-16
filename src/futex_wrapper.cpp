#include "futex_wrapper.h"

#ifdef _MSC_VER
#include <windows.h>
#else
#include <linux/futex.h>
#include <sys/syscall.h> // SYS_futex
#include <sys/time.h> // struct timespec
#include <unistd.h> // syscall()
#include <limits.h>
#endif

namespace threadkit {

#ifdef _MSC_VER

bool FutexWait(uint32_t* addr, uint32_t value, const TimeVal* timeout) {
    const DWORD ms = (timeout) ?
        (timeout->tv_sec * 1000 + timeout->tv_usec / 1000) :
        INFINITE;
    return WaitOnAddress(addr, &value, sizeof(uint32_t), ms);
}

void FutexWakeOne(uint32_t* addr) {
    WakeByAddressSingle(addr);
}

void FutexWakeAll(uint32_t* addr) {
    WakeByAddressAll(addr);
}

#else

// refer to http://locklessinc.com/articles/futex_cheat_sheet/

bool FutexWait(uint32_t* addr, uint32_t value, const TimeVal* timeout) {
    int ret;
    if (timeout) {
        const struct timespec ts = {
            .tv_sec = (time_t)timeout->tv_sec,
            .tv_nsec = timeout->tv_usec * 1000,
        };
        ret = syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, value, &ts, nullptr, 0);
    } else {
        ret = syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, value, nullptr, nullptr, 0);
    }

    return (ret != -1);
}

void FutexWakeOne(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

void FutexWakeAll(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, INT_MAX, nullptr, nullptr, 0);
}

#endif

}
