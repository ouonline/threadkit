#include <iostream>
using namespace std;

#include <unistd.h>

#include "thread_pool.hpp"
using namespace utils;

class TestThreadTask : public JoinableThreadTask {

public:
    TestThreadTask(const string& msg) {
        m_is_finished = false;
        m_msg = msg;
    }

protected:
    void Process() override {
        cout << m_msg << endl;
        m_is_finished = true;
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
    tp.AddThread(5);

    auto task = make_shared<TestThreadTask>("Hello, world!");
    tp.AddTask(task);

    tp.DelThread(2);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    tp.AddThread(5);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    task->Join();

    return 0;
}
