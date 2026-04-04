#ifndef __THREADKIT_COMMON_H__
#define __THREADKIT_COMMON_H__

#include <stdint.h>

namespace threadkit {

// L1 cacheline size of most CPUs
static constexpr uint32_t CACHELINE_SIZE = 64;

#if defined(__x86_64__) || defined(__i386__)
#define cpu_relax() asm volatile("pause" ::: "memory")
#elif defined(__aarch64__) || defined(__arm64__) || defined(__arm__)
#define cpu_relax() asm volatile("yield" ::: "memory")
#else
#define cpu_relax()
#endif

}

#endif
