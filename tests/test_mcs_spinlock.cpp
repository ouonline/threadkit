#include "threadkit/mcs_spinlock.h"
using namespace threadkit;

#include <thread>
#include <iostream>
#include <vector>
#include <unistd.h>
using namespace std;

#undef NDEBUG
#include <assert.h>

#define N 10
#define NR 100

static void TestMCS() {
    vector<std::thread> thread_list;
    thread_list.reserve(N);

    int counter = 0;
    MCSSpinLock mcs_lock;

    for (int i = 0; i < N; ++i) {
        thread_list.emplace_back([&counter, &mcs_lock]() -> void {
            MCSSpinLock::Node node;
            for (int j = 0; j < NR; ++j) {
                mcs_lock.Lock(&node);
                ++counter;
                mcs_lock.Unlock(&node);
            }
        });
    }

    for (int i = 0; i < N; ++i) {
        thread_list[i].join();
    }

    assert(counter == N * NR);
}

int main(void) {
    TestMCS();
    return 0;
}
