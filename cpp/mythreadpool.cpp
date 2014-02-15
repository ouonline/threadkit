#include <unistd.h>
#include "mythreadpool.hpp"
using namespace myutils;

static void* worker_func(void* arg)
{
    MyThreadTaskQueue* q = (MyThreadTaskQueue*)arg;

    while (true) {
        pthread_mutex_lock(&q->mutex);

        while (q->tasklist.empty())
            pthread_cond_wait(&q->cond, &q->mutex);

        shared_ptr<MyThreadTask> t = q->tasklist.front();
        q->tasklist.pop();

        pthread_mutex_unlock(&q->mutex);

        if (!t)
            break;

        t->run();
    }

    return nullptr;
}

void MyThreadPool::doAddTask(const shared_ptr<MyThreadTask>& t)
{
    pthread_mutex_lock(&m_queue.mutex);
    m_queue.tasklist.push(t);
    pthread_cond_signal(&m_queue.cond);
    pthread_mutex_unlock(&m_queue.mutex);
}

bool MyThreadPool::addTask(const shared_ptr<MyThreadTask>& t)
{
    if (!m_valid)
        return false;

    if (!t)
        return false;

    doAddTask(t);
    return true;
}

MyThreadPool::MyThreadPool(int num)
{
    m_valid = false;

    if (num <= 0)
        num = sysconf(_SC_NPROCESSORS_CONF) * 2;

    for (int i = 0; i < num; ++i) {
        pthread_t pid;
        int err = pthread_create(&pid, nullptr, worker_func, &m_queue);
        if (!err)
            m_pidlist.push_back(pid);
    }

    if (m_pidlist.size() > 0)
        m_valid = true;
}

MyThreadPool::~MyThreadPool()
{
    m_valid = false; // cannot addTask() any more

    for (size_t i = 0; i < m_pidlist.size(); ++i)
        doAddTask(shared_ptr<MyThreadTask>(nullptr));

    // waiting for remaining tasks to complete
    for (auto i = m_pidlist.begin(); i != m_pidlist.end(); ++i)
        pthread_join(*i, nullptr);
}
