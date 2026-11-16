#ifndef __THREADKIT_FUTEX_WRAPPER_H__
#define __THREADKIT_FUTEX_WRAPPER_H__

#include "threadkit/timeval.h"
#include <stdint.h>

namespace threadkit {

bool FutexWait(uint32_t* addr, uint32_t value,
               const TimeVal* timeout = nullptr);
void FutexWakeOne(uint32_t* addr);
void FutexWakeAll(uint32_t* addr);

}

#endif
