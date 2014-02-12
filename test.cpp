#include <iostream>
using namespace std;

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

    tp.addTask(std::shared_ptr<MyThreadTask>(new TestThreadTask("Hello, world!")));

    return 0;
}
