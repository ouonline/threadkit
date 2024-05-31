#include "threadkit/spsc_queue.h"
#include <thread>
#include <iostream>
using namespace std;
using namespace threadkit;

#undef NDEBUG
#include <assert.h>

static void TestPushPop() {
    constexpr int expected_value = 10;

    SPSCQueue<int> q;
    q.Push(expected_value);

    int ret;
    assert(q.Pop(&ret));
    assert(ret == expected_value);

    assert(!q.Pop());
}

#define N 5

static void TestSPSC() {
    SPSCQueue<int> q;

    std::thread producer([&q]() -> void {
        for (int i = 0; i < N; ++i) {
            q.Push(i);
            cout << "push " << i << endl;
        }
    });

    std::thread consumer([&q]() -> void {
        int expected_value = 0;
        while (true) {
            int ret;
            if (!q.Pop(&ret)) {
                continue;
            }
            cout << "pop " << ret << endl;
            assert(ret == expected_value);
            ++expected_value;
            if (expected_value == N) {
                assert(!q.Pop());
                break;
            }
        }
    });

    producer.join();
    consumer.join();
}

int main(void) {
    TestPushPop();
    TestSPSC();
    return 0;
}
