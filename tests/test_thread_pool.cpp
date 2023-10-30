#include <iostream>
#include <string>
#include <atomic>
using namespace std;

#include "threadkit/event_count.h"
#include "threadkit/threadpool.h"
using namespace threadkit;

class JoinableTask final : public ThreadTask {
public:
    JoinableTask(const string& msg, atomic<uint32_t>* nr_finished, uint32_t expected, EventCount* cond)
        : m_expected(expected), m_msg(msg), m_nr_finished(nr_finished), m_cond(cond) {}
    shared_ptr<ThreadTask> Run(uint32_t) override {
        cout << "tid [" << std::this_thread::get_id() << "], msg -> " << m_msg << endl;
        auto count = m_nr_finished->fetch_add(1, std::memory_order_acq_rel) + 1;
        if (count >= m_expected) {
            m_cond->NotifyOne();
        }
        return shared_ptr<ThreadTask>();
    }

private:
    const uint32_t m_expected;
    string m_msg;
    atomic<uint32_t>* m_nr_finished;
    EventCount* m_cond;
};

static void EmptyDeleter(ThreadTask*) {}

static void TestTask(void) {
    ThreadPool tp;
    tp.Init(2);

    EventCount cond;
    atomic<uint32_t> nr_finished(0);
    JoinableTask task("Hello, world!", &nr_finished, 1, &cond);
    tp.AddTask(shared_ptr<ThreadTask>(&task, EmptyDeleter));
    cond.Wait([&nr_finished]() -> bool {
        return (nr_finished.load(std::memory_order_acquire) >= 1);
    });
}

#define N 2

static void TestAsync(void) {
    FixedThreadPool tp;
    tp.Init(N);

    EventCount cond;
    atomic<uint32_t> nr_finished(0);
    tp.ParallelRunAsync([&nr_finished, &cond](uint32_t thread_idx) -> void {
        cout << "thread [" << thread_idx << "] finished." << endl;
        nr_finished.fetch_add(1, std::memory_order_acq_rel);
        cond.NotifyOne();
    });
    cond.Wait([&nr_finished]() -> bool {
        return (nr_finished.load(std::memory_order_acquire) >= N);
    });
}

static void TestSync(void) {
    FixedThreadPool tp;
    tp.Init(N);

    tp.ParallelRun([](uint32_t thread_idx) -> void {
        cout << "thread [" << thread_idx << "] finished." << endl;
    });
}

struct {
    const char* name;
    void (*func)(void);
} g_test_suite[] = {
    {"task", TestTask},
    {"async", TestAsync},
    {"sync", TestSync},
    {nullptr, nullptr},
};

int main(void) {
    for (uint32_t i = 0; g_test_suite[i].name; ++i) {
        cout << "----- " << g_test_suite[i].name << " -----" << endl;
        g_test_suite[i].func();
    }

    return 0;
}
