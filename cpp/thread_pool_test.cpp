#include <iostream>
using namespace std;

#include <unistd.h>

#include "thread_pool.hpp"
using namespace utils;

class TestThreadTask : public ThreadTask {

public:
    TestThreadTask(const string& msg) {
        m_msg = msg;
    }

    void Run() {
        cout << m_msg << endl;
    }

private:
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
