#include <thread>
#include <vector>
#include <iostream>
using namespace std;

#undef NDEBUG
#include <assert.h>
#include "utils.h"

#define private public
#include "threadkit/scheduler.h"
using namespace threadkit;

static void TestPush() {
    Scheduler sched;
    assert(sched.Init(2));
    sched.Start();

    MPSCQueue::Node node1, node2;
    sched.Push(&node1);
    sched.Push(&node2);

    sched.Stop();
}

static void TestPop() {
    Scheduler sched;
    assert(sched.Init(1));
    sched.Start();

    MPSCQueue::Node node1;
    sched.Push(&node1);

    auto ret_node = sched.Pop(0);
    assert(ret_node == &node1);

    sched.Stop();
}

static void TestTryPop() {
    Scheduler sched;
    assert(sched.Init(1));
    sched.Start();

    MPSCQueue::Node node1;
    sched.Push(&node1);

    auto ret_node = sched.TryPop(0);
    assert(ret_node == &node1);

    ret_node = sched.TryPop(0);
    assert(ret_node == nullptr);

    sched.Stop();
}

#define N 5

static void Thread0(Scheduler* sched, uint32_t* run_count, bool* thread1_alive) {
    while (true) {
        // invoke Pop() to trigger request stealing
        sched->Pop(0);
        sched->m_push_idx.store(0, std::memory_order_seq_cst);
        SleepSec(1);
        ++(*run_count);
        if (!(*thread1_alive)) {
            break;
        }
    }
}

static void Thread1(const vector<MPSCQueue::Node>* node_list, Scheduler* sched,
                    uint32_t* run_count, bool* thread1_alive) {
    while (true) {
        auto node = sched->Pop(1);
        uint32_t ret_idx = N;
        for (uint32_t i = 0; i < N; ++i) {
            if (node == &node_list->at(i)) {
                ret_idx = i;
                break;
            }
        }
        assert(ret_idx < N);
        cout << "node from queue0" << endl;
        ++(*run_count);
        if (*run_count == 2) {
            break;
        }
    }

    *thread1_alive = false;
}

static void TestStealReq() {
    Scheduler sched;
    assert(sched.Init(2));
    sched.Start();

    vector<MPSCQueue::Node> node_list(N);

    for (uint32_t i = 0; i < N; ++i) {
        sched.m_push_idx.store(0, std::memory_order_seq_cst);
        sched.Push(&node_list[i]);
    }

    bool thread1_alive = true;

    uint32_t t0_run_count = 0;
    std::thread t0(Thread0, &sched, &t0_run_count, &thread1_alive);

    uint32_t t1_run_count = 0;
    std::thread t1(Thread1, &node_list, &sched, &t1_run_count, &thread1_alive);

    t0.join();
    t1.join();

    assert(t0_run_count > 0);
    assert(t1_run_count > 0);

    sched.Stop();
}

#undef N

static const struct {
    const char* name;
    void (*func)(void);
} g_test_suite[] = {
    {"TestPush", TestPush},
    {"TestPop", TestPop},
    {"TestTryPop", TestTryPop},
    {"TestStealReq", TestStealReq},
    {nullptr, nullptr},
};

int main(void) {
    for (uint32_t i = 0; g_test_suite[i].name; ++i) {
        cout << "----- " << g_test_suite[i].name << " -----" << endl;
        g_test_suite[i].func();
    }
    return 0;
}
