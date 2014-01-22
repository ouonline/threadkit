#include <iostream>
using namespace std;

#include "threadpool.hpp"

class TestTaskEntity : public ThreadTaskEntity {

    public:

        TestTaskEntity(const string& msg)
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
    ThreadPool tp(5);

    tp.addTask(std::shared_ptr<ThreadTaskEntity>(new TestTaskEntity("Hello, world!")));

    return 0;
}
