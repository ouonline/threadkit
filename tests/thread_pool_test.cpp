#include <iostream>
using namespace std;

#include <unistd.h>

#include "threadpool/threadpool.h"
using namespace utils;

class TestThreadTask : public JoinableThreadTask {

public:
    TestThreadTask(const string& msg) {
        m_is_finished = false;
        m_msg = msg;
    }

protected:
    ThreadTaskInfo Process() override {
        cout << "tid[" << pthread_self() << "], msg -> " << m_msg << endl;
        m_is_finished = true;
        return ThreadTaskInfo();
    }
    bool IsFinished() const override {
        return m_is_finished;
    }

private:
    bool m_is_finished;
    string m_msg;
};

int main(void) {
    ThreadPool tp;

    tp.AddThread(8);

    TestThreadTask task("Hello, world!");
    tp.AddTask(ThreadTaskInfo(&task));

    vector<TestThreadTask> tasks;
    tasks.emplace_back(TestThreadTask("a"));
    tasks.emplace_back(TestThreadTask("b"));
    tasks.emplace_back(TestThreadTask("c"));
    tasks.emplace_back(TestThreadTask("d"));
    tasks.emplace_back(TestThreadTask("e"));

    tp.BatchAddTask([&tasks] (const function<void (const ThreadTaskInfo&)>& add_task) -> void {
        for (auto iter = tasks.begin(); iter != tasks.end(); ++iter) {
            add_task(ThreadTaskInfo(&(*iter)));
        }
    });

    tp.DelThread(2);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    tp.AddThread(5);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    task.Join();
    for (auto x = tasks.begin(); x != tasks.end(); ++x) {
        x->Join();
    }

    return 0;
}
