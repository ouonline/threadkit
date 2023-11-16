#include <iostream>
using namespace std;

#include "threadkit/event_count.h"
#include "threadkit/threadpool.h"
using namespace threadkit;

class JoinableTask final : public ThreadTask {
public:
    JoinableTask(bool* finished, EventCount* cond) : m_finished(finished), m_cond(cond) {}
    ThreadTask* Run(uint32_t) override {
        cout << "task [" << std::this_thread::get_id() << "]: Hello, world!" << endl;
        *m_finished = true;
        m_cond->NotifyOne();
        return nullptr;
    }
    void DeleteCallback() {
        cout << "task [" << std::this_thread::get_id() << "] is deleted." << endl;
    }

private:
    bool* m_finished;
    EventCount* m_cond;
};

static void TestDeleter(ThreadTask* t) {
    auto tt = static_cast<JoinableTask*>(t);
    tt->DeleteCallback();
}

static void TestTask(void) {
    ThreadPool tp;
    tp.Init(2);

    EventCount cond;
    bool finished = false;

    JoinableTask task(&finished, &cond);
    task.deleter = TestDeleter;
    tp.AddTask(&task);

    cond.Wait([&finished]() -> bool {
        return finished;
    });
}

#define N 10

static void TestAsync(void) {
    StaticThreadPool tp;
    tp.Init(N);

    EventCount cond;
    atomic<uint32_t> nr_finished(0);
    tp.ParallelRunAsync([&nr_finished, &cond](uint32_t thread_idx) -> void {
        cout << "thread [" << thread_idx << "] finished." << endl;
        auto value = nr_finished.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (value >= N) {
            cond.NotifyOne();
        }
    });
    cond.Wait([&nr_finished]() -> bool {
        return (nr_finished.load(std::memory_order_acquire) >= N);
    });
}

static void TestSync(void) {
    StaticThreadPool tp;
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
