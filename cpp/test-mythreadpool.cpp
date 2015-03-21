#include <iostream>
using namespace std;

#include <unistd.h>

#include "mythreadpool.hpp"
using namespace myutils;

class TestThreadTask : public MyThreadTask {

    public:

        TestThreadTask(const string& msg)
        {
            m_msg = msg;
        }

        void run()
        {
            cout << m_msg << endl;
        }

    private:

        string m_msg;
};

int main(void)
{
    MyThreadPool tp(5);

    tp.addTask(make_shared<TestThreadTask>("Hello, world!"));

    tp.delThread(2);
    sleep(1);
    cout << "thread num = " << tp.threadNum() << endl;

    tp.addThread(5);
    sleep(1);
    cout << "thread num = " << tp.threadNum() << endl;

    return 0;
}
