#ifndef __THREADKIT_FUTEX_WRAPPER_H__
#define __THREADKIT_FUTEX_WRAPPER_H__

#include <stdint.h>
#include "threadkit/timeval.h"

namespace threadkit {

void FutexWait(uint32_t* addr, uint32_t value, const TimeVal* timeout = nullptr);
void FutexWakeOne(uint32_t* addr);
void FutexWakeAll(uint32_t* addr);

}

#endif
