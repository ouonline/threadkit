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

    bool is_empty = false;
    auto node = q.Pop(&is_empty);
    assert(!node);
    assert(is_empty);

    auto node1 = new TestNode(23333);
    is_empty = q.Push(node1);
    assert(is_empty);

    node = q.Pop(&is_empty);
    assert(node);
    auto node2 = static_cast<TestNode*>(node);
    assert(node2->value == 23333);
    assert(!is_empty);

    delete node1;
}

int main(void) {
    TestPush();
    TestPop();
    return 0;
}
