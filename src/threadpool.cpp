#include "threadkit/threadpool.h"
#include <thread>
using namespace std;

namespace threadkit {

shared_ptr<ThreadTask> JoinableThreadTask::Run() {
    shared_ptr<ThreadTask> next_task;
    if (!IsFinished()) {
        m_mutex.lock();
        next_task = Process();
        m_mutex.unlock();
        m_cond.notify_one();
    }
    return next_task;
}

void JoinableThreadTask::Join() {
    std::unique_lock<std::mutex> lck(m_mutex);
    while (!IsFinished()) {
        m_cond.wait(lck);
    }
}

/* -------------------------------------------------------------------------- */

void ThreadPool::ThreadFunc(ThreadPool* tp) {
    auto q = &(tp->m_queue);
    while (true) {
        auto task = q->Pop();
        if (!task) {
            tp->m_thread_lock.lock();
            --tp->m_thread_num;
            if (tp->m_thread_num == 0) {
                tp->m_thread_cond.notify_one();
            }
            tp->m_thread_lock.unlock();
            break;
        }

        do {
            task = task->Run();
        } while (task);
    }
}

void ThreadPool::DoAddTask(const shared_ptr<ThreadTask>& task) {
    m_queue.Push(task);
}

void ThreadPool::AddTask(const shared_ptr<ThreadTask>& task) {
    if (task) {
        DoAddTask(task);
    }
}

void ThreadPool::AddThread(unsigned int num) {
    for (unsigned int i = 0; i < num; ++i) {
        std::thread t(ThreadFunc, this);
        t.detach();
        m_thread_lock.lock();
        ++m_thread_num;
        m_thread_lock.unlock();
    }
}

void ThreadPool::DelThread(unsigned int num) {
    if (num > m_thread_num) {
        num = m_thread_num;
    }

    shared_ptr<ThreadTask> dummy_task;
    for (unsigned int i = 0; i < num; ++i) {
        DoAddTask(dummy_task);
    }
}

ThreadPool::~ThreadPool() {
    DelThread(m_thread_num);

    // waiting for remaining task(s) to complete
    std::unique_lock<std::mutex> lck(m_thread_lock);
    while (m_thread_num > 0) {
        m_thread_cond.wait(lck);
    }
}

}
