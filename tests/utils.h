#ifndef __THREADKIT_TESTS_UTILS_H__
#define __THREADKIT_TESTS_UTILS_H__

#ifdef _MSC_VER
#include <windows.h>
static inline void SleepSec(int sec) {
    Sleep(sec * 1000);
}
#else
#include <unistd.h>
static inline void SleepSec(int sec) {
    sleep(sec);
}
#endif

#endif
