#include "threadkit/mpsc_queue.h"
using namespace std;
using namespace threadkit;

#undef NDEBUG
#include <assert.h>

struct TestNode final : public MPSCQueue::Node {
    TestNode(int v) : value(v) {}
    int value;
};

static void TestPush() {
    MPSCQueue q;

    auto node1 = new TestNode(23333);
    bool is_empty = q.Push(node1);
    assert(is_empty);

    auto node2 = new TestNode(12345);
    is_empty = q.Push(node2);
    assert(!is_empty);

    delete node1;
    delete node2;
}

static void TestPop() {
    MPSCQueue q;

    pair<MPSCQueue::Node*, bool> ret_pair = q.Pop();
    assert(!ret_pair.first);
    assert(ret_pair.second);

    bool is_empty;
    auto node1 = new TestNode(23333);
    is_empty = q.Push(node1);
    assert(is_empty);

    ret_pair = q.Pop();
    assert(ret_pair.first);
    assert(!ret_pair.second);
    auto node2 = static_cast<TestNode*>(ret_pair.first);
    assert(node2->value == 23333);

    delete node1;
}

int main(void) {
    TestPush();
    TestPop();
    return 0;
}
