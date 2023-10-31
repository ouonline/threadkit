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
    ThreadTask* Run(uint32_t) override {
        cout << "task [" << std::this_thread::get_id() << "], msg -> " << m_msg << endl;
        auto count = m_nr_finished->fetch_add(1, std::memory_order_acq_rel) + 1;
        if (count >= m_expected) {
            m_cond->NotifyOne();
        }
        return nullptr;
    }
    void DeleteCallback() {
        cout << "task [" << std::this_thread::get_id() << "] is deleted." << endl;
    }

private:
    const uint32_t m_expected;
    string m_msg;
    atomic<uint32_t>* m_nr_finished;
    EventCount* m_cond;
};

static void TestDeleter(ThreadTask* t) {
    auto tt = static_cast<JoinableTask*>(t);
    tt->DeleteCallback();
}

static void TestTask(void) {
    ThreadPool tp;
    tp.Init(2);

    constexpr uint32_t expected = 1;
    EventCount cond;
    atomic<uint32_t> nr_finished(0);

    JoinableTask task("Hello, world!", &nr_finished, expected, &cond);
    task.deleter = TestDeleter;
    tp.AddTask(&task);

    cond.Wait([&nr_finished, expected]() -> bool {
        return (nr_finished.load(std::memory_order_acquire) >= expected);
    });
}

struct {
    const char* name;
    void (*func)(void);
} g_test_suite[] = {
    {"task", TestTask},
    {nullptr, nullptr},
};

int main(void) {
    for (uint32_t i = 0; g_test_suite[i].name; ++i) {
        cout << "----- " << g_test_suite[i].name << " -----" << endl;
        g_test_suite[i].func();
    }

    return 0;
}
