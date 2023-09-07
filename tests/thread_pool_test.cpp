#include <iostream>
#include <chrono>
using namespace std;

#include "threadkit/threadpool.h"
using namespace threadkit;

class TestThreadTask : public JoinableThreadTask {
public:
    TestThreadTask(const string& msg) {
        m_is_finished = false;
        m_msg = msg;
    }

protected:
    shared_ptr<ThreadTask> Process() override {
        cout << "tid[" << std::this_thread::get_id() << "], msg -> " << m_msg << endl;
        m_is_finished = true;
        return shared_ptr<ThreadTask>();
    }
    bool IsFinished() const override {
        return m_is_finished;
    }

private:
    bool m_is_finished;
    string m_msg;
};

static void EmptyDeleter(ThreadTask*) {}

int main(void) {
    ThreadPool tp;

    tp.Init(8);

    TestThreadTask task("Hello, world!");
    tp.AddTask(shared_ptr<ThreadTask>(&task, EmptyDeleter));
    task.Join();

    return 0;
}
