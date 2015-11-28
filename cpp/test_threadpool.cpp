#include <iostream>
using namespace std;

#include <unistd.h>

#include "threadpool.hpp"
using namespace utils;

class TestThreadTask : public ThreadTask {

    public:

        TestThreadTask(const string& msg)
        {
            m_msg = msg;
        }

        void Run()
        {
            cout << m_msg << endl;
        }

    private:

        string m_msg;
};

int main(void)
{
    ThreadPool tp(5);

    tp.AddTask(make_shared<TestThreadTask>("Hello, world!"));

    tp.DelThread(2);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    tp.AddThread(5);
    sleep(1);
    cout << "thread num = " << tp.ThreadNum() << endl;

    return 0;
}
