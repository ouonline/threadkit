#ifndef __THREADKIT_UTILS_H__
#define __THREADKIT_UTILS_H__

#ifndef likely
#ifdef _MSC_VER
#define likely(x) (x)
#else
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#endif

#ifndef unlikely
#ifdef _MSC_VER
#define unlikely(x) (x)
#else
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#endif

#endif
