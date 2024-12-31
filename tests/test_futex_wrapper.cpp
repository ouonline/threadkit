#include <time.h>
#include <cstdio>
#include "../src/futex_wrapper.h"
using namespace threadkit;

#undef NDEBUG
#include <assert.h>

static void TestTimeout() {
    const TimeVal timeout = {
        .tv_sec = 1,
        .tv_usec = 0,
    };

    time_t begin_ts = time(nullptr);
    uint32_t addr = 10;
    FutexWait(&addr, addr, &timeout);
    time_t end_ts = time(nullptr);
    assert(end_ts - begin_ts >= timeout.tv_sec);
}

int main(void) {
    TestTimeout();
    return 0;
}
