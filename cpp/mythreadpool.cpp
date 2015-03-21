#include <unistd.h>

#include "mythreadpool.hpp"
using namespace std;

namespace myutils {

void* MyThreadPool::thread_worker(void* arg)
{
    auto tp = (MyThreadPool*)arg;
    auto q = &(tp->m_queue);

    while (true) {
        pthread_mutex_lock(&q->mutex);

        while (q->tasklist.empty())
            pthread_cond_wait(&q->cond, &q->mutex);

        auto t = q->tasklist.front();
        q->tasklist.pop();

        pthread_mutex_unlock(&q->mutex);

        if (!t) {
            pthread_mutex_lock(&tp->m_thread_lock);
            tp->m_thread_list.erase(pthread_self());
            pthread_mutex_unlock(&tp->m_thread_lock);
            pthread_cond_signal(&tp->m_thread_cond);

            break;
        }

        t->run();
    }

    return nullptr;
}

void MyThreadPool::doAddTask(const shared_ptr<MyThreadTask>& t)
{
    pthread_mutex_lock(&m_queue.mutex);
    m_queue.tasklist.push(t);
    pthread_mutex_unlock(&m_queue.mutex);
    pthread_cond_signal(&m_queue.cond);
}

bool MyThreadPool::addTask(const shared_ptr<MyThreadTask>& t)
{
    if (m_thread_list.empty())
        return false;

    if (!t)
        return false;

    doAddTask(t);
    return true;
}

void MyThreadPool::doAddThread()
{
    pthread_t pid;
    if (pthread_create(&pid, nullptr, thread_worker, this) == 0) {
        pthread_mutex_lock(&m_thread_lock);
        m_thread_list.insert(pid);
        pthread_mutex_unlock(&m_thread_lock);
    }
}

void MyThreadPool::addThread(unsigned int num)
{
    for (unsigned int i = 0; i < num; ++i)
        doAddThread();
}

void MyThreadPool::doDelThread()
{
    doAddTask(shared_ptr<MyThreadTask>(nullptr));
}

void MyThreadPool::delThread(unsigned int num)
{
    if (num > m_thread_list.size())
        num = m_thread_list.size();

    for (unsigned int i = 0; i < num; ++i)
        doDelThread();
}

MyThreadPool::MyThreadPool(unsigned int num)
{
    if (num == 0)
        num = sysconf(_SC_NPROCESSORS_CONF) - 1;

    pthread_mutex_init(&m_thread_lock, nullptr);
    pthread_cond_init(&m_thread_cond, nullptr);

    for (unsigned int i = 0; i < num; ++i)
        doAddThread();
}

MyThreadPool::~MyThreadPool()
{
    unsigned int num = m_thread_list.size();

    for (unsigned int i = 0; i < num; ++i)
        doDelThread();

    // waiting for remaining task(s) to complete
    pthread_mutex_lock(&m_thread_lock);
    while (!m_thread_list.empty())
        pthread_cond_wait(&m_thread_cond, &m_thread_lock);
    pthread_mutex_unlock(&m_thread_lock);

    pthread_cond_destroy(&m_thread_cond);
    pthread_mutex_destroy(&m_thread_lock);
}

}
