#ifndef __THREADKIT_COMMON_H__
#define __THREADKIT_COMMON_H__

namespace threadkit {

// L1 cacheline size of most CPUs
static constexpr unsigned int CACHELINE_SIZE = 64;

inline void CpuRelax() {
#if defined(__x86_64__) || defined(__i386__)
    asm volatile("pause" ::: "memory");
#elif defined(__aarch64__) || defined(__arm64__) || defined(__arm__)
    asm volatile("yield" ::: "memory");
#endif
}

}

#endif
