#include "threadkit/threadpool.h"
using namespace std;

namespace outils {

JoinableThreadTask::JoinableThreadTask() {
    pthread_mutex_init(&m_mutex, nullptr);
    pthread_cond_init(&m_cond, nullptr);
}

JoinableThreadTask::~JoinableThreadTask() {
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
}

shared_ptr<ThreadTask> JoinableThreadTask::Run() {
    shared_ptr<ThreadTask> next_task;
    if (!IsFinished()) {
        pthread_mutex_lock(&m_mutex);
        next_task = Process();
        pthread_mutex_unlock(&m_mutex);
        pthread_cond_signal(&m_cond);
    }
    return next_task;
}

void JoinableThreadTask::Join() {
    pthread_mutex_lock(&m_mutex);
    while (!IsFinished()) {
        pthread_cond_wait(&m_cond, &m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);
}

/* -------------------------------------------------------------------------- */

void* ThreadPool::ThreadFunc(void* arg) {
    auto tp = (ThreadPool*)arg;
    auto q = &(tp->m_queue);

    while (true) {
        auto task = q->Pop();
        if (!task) {
            pthread_mutex_lock(&tp->m_thread_lock);
            --tp->m_thread_num;
            if (tp->m_thread_num == 0) {
                pthread_cond_signal(&tp->m_thread_cond);
            }
            pthread_mutex_unlock(&tp->m_thread_lock);
            break;
        }

        do {
            task = task->Run();
        } while (task);
    }

    return nullptr;
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
    pthread_t pid;
    for (unsigned int i = 0; i < num; ++i) {
        if (pthread_create(&pid, nullptr, ThreadFunc, this) == 0) {
            pthread_detach(pid);
            pthread_mutex_lock(&m_thread_lock);
            ++m_thread_num;
            pthread_mutex_unlock(&m_thread_lock);
        }
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

ThreadPool::ThreadPool() {
    m_thread_num = 0;
    pthread_mutex_init(&m_thread_lock, nullptr);
    pthread_cond_init(&m_thread_cond, nullptr);
}

ThreadPool::~ThreadPool() {
    DelThread(m_thread_num);

    // waiting for remaining task(s) to complete
    pthread_mutex_lock(&m_thread_lock);
    while (m_thread_num > 0) {
        pthread_cond_wait(&m_thread_cond, &m_thread_lock);
    }
    pthread_mutex_unlock(&m_thread_lock);

    pthread_cond_destroy(&m_thread_cond);
    pthread_mutex_destroy(&m_thread_lock);
}

}
