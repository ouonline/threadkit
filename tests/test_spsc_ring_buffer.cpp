#include "threadkit/spsc_ring_buffer.h"
using namespace threadkit;

#undef NDEBUG
#include <assert.h>

static void test_push() {
    SPSCRingBuffer<int> q(2);
    assert(q.IsEmpty());

    assert(q.Push(6));
    assert(!q.IsEmpty());
    assert(!q.IsFull());

    assert(q.Push(8));
    assert(!q.IsEmpty());
    assert(q.IsFull());

    assert(!q.Push(10));
}

static void test_pop() {
    int values[] = {100, 200, 300};
    const uint32_t N = sizeof(values) / sizeof(int);

    SPSCRingBuffer<int> q(N);
    assert(q.IsEmpty());

    for (int i = 0; i < N; ++i) {
        assert(q.Push(values[i]));
        assert(!q.IsEmpty());
        if (i != N - 1) {
            assert(!q.IsFull());
        } else {
            assert(q.IsFull());
        }
    }
    assert(!q.Push(400));

    for (int i = 0; i < N; ++i) {
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

int main(void) {
    test_push();
    test_pop();
    return 0;
}
