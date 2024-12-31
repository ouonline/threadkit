#ifndef __THREADKIT_TIMEVAL_H__
#define __THREADKIT_TIMEVAL_H__

#ifdef _MSC_VER
#include <winsock.h>
#else
#include <time.h>
#endif

namespace threadkit {

typedef struct timeval TimeVal;

}

#endif
