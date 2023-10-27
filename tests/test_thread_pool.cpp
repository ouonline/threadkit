#include <iostream>
#include <string>
using namespace std;

#include "threadkit/threadpool.h"
using namespace threadkit;

class JoinableTask final : public ThreadTask {
public:
    JoinableTask(const string& msg, Event* event) : m_msg(msg), m_event(event) {}
    shared_ptr<ThreadTask> Run(uint32_t) override {
        cout << "tid[" << std::this_thread::get_id() << "], msg -> " << m_msg << endl;
        m_event->Finish();
        return shared_ptr<ThreadTask>();
    }

private:
    string m_msg;
    Event* m_event;
};

static void EmptyDeleter(ThreadTask*) {}

static void TestTask(void) {
    ThreadPool tp;
    tp.Init(2);

    Event event(1);
    JoinableTask task("Hello, world!", &event);
    tp.AddTask(shared_ptr<ThreadTask>(&task, EmptyDeleter));
    event.Wait();
}

#define N 2

static void TestAsync(void) {
    FixedThreadPool tp;
    tp.Init(N);

    Event event(N);
    tp.ParallelRunAsync([&event](uint32_t thread_idx) -> void {
        cout << "thread [" << thread_idx << "] finished." << endl;
        event.Finish();
    });
    event.Wait();
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
