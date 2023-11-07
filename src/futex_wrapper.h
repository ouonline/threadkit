#ifndef __THREADKIT_FUTEX_WRAPPER_H__
#define __THREADKIT_FUTEX_WRAPPER_H__

#include <stdint.h>

namespace threadkit {

void FutexWait(uint32_t*, uint32_t);
void FutexWakeOne(uint32_t*);
void FutexWakeAll(uint32_t*);

}

#endif
