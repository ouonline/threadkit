#include "threadkit/spsc_bounded_queue.h"
using namespace threadkit;

#include <thread>
#include <iostream>
using namespace std;

#undef NDEBUG
#include <assert.h>

static void TestPush() {
    SPSCBoundedQueue<int> q(2);
    assert(q.IsEmpty());

    assert(q.Push(6));
    assert(!q.IsEmpty());
    assert(!q.IsFull());

    assert(q.Push(8));
    assert(!q.IsEmpty());
    assert(q.IsFull());

    assert(!q.Push(10));
}

static void TestPop() {
    int values[] = {100, 200, 300};
    const uint32_t N = sizeof(values) / sizeof(int);

    SPSCBoundedQueue<int> q(N);
    assert(q.IsEmpty());

    for (uint32_t i = 0; i < N; ++i) {
        assert(q.Push(values[i]));
        assert(!q.IsEmpty());
        if (i != N - 1) {
            assert(!q.IsFull());
        } else {
            assert(q.IsFull());
        }
    }
    assert(!q.Push(400));

    for (uint32_t i = 0; i < N; ++i) {
        int ret = 0;
        assert(q.Pop(&ret));
        if (i != N - 1) {
            assert(!q.IsEmpty());
        } else {
            assert(q.IsEmpty());
        }
        assert(!q.IsFull());
        assert(ret == values[i]);
    }
}

#define N 5

static void TestSPSC() {
    SPSCBoundedQueue<int> q(N);

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
    TestPush();
    TestPop();
    TestSPSC();
    return 0;
}
