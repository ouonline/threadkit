#include "futex_wrapper.h"

namespace threadkit {

#ifdef _MSC_VER

#include <windows.h>

void FutexWait(uint32_t* addr, uint32_t value) {
    WaitOnAddress(addr, &value, sizeof(uint32_t), INFINITE);
}

void FutexWakeOne(uint32_t* addr) {
    WakeByAddressSingle(addr);
}

void FutexWakeAll(uint32_t* addr) {
    WakeByAddressAll(addr);
}

#else

#include <linux/futex.h>
#include <sys/syscall.h> // SYS_futex
#include <unistd.h> // syscall()
#include <climits>

// refer to http://locklessinc.com/articles/futex_cheat_sheet/

void FutexWait(uint32_t* addr, uint32_t value) {
	syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, value, nullptr, nullptr, 0);
}

void FutexWakeOne(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

void FutexWakeAll(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, INT_MAX, nullptr, nullptr, 0);
}

#endif

}
