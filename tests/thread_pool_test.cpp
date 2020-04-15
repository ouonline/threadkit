#include <iostream>
using namespace std;

#include <unistd.h>

#include "threadkit/threadpool.h"
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

    tp.DelThread(2);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    tp.AddThread(5);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    task.Join();

    return 0;
}
