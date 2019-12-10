#include "threadpool.h"
using namespace std;

namespace utils {

JoinableThreadTask::JoinableThreadTask() {
    pthread_mutex_init(&m_mutex, nullptr);
    pthread_cond_init(&m_cond, nullptr);
}

JoinableThreadTask::~JoinableThreadTask() {
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
}

ThreadTaskInfo JoinableThreadTask::Run() {
    ThreadTaskInfo info;
    if (!IsFinished()) {
        pthread_mutex_lock(&m_mutex);
        info = Process();
        pthread_mutex_unlock(&m_mutex);
        pthread_cond_signal(&m_cond);
    }
    return info;
}

void JoinableThreadTask::Join() {
    pthread_mutex_lock(&m_mutex);
    while (!IsFinished()) {
        pthread_cond_wait(&m_cond, &m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);
}

/* -------------------------------------------------------------------------- */

void* ThreadPool::ThreadWorker(void* arg) {
    auto tp = (ThreadPool*)arg;
    auto q = &(tp->m_queue);

    while (true) {
        auto info = q->Pop();
        if (!info.task) {
            pthread_mutex_lock(&tp->m_thread_lock);
            --tp->m_thread_num;
            pthread_mutex_unlock(&tp->m_thread_lock);
            pthread_cond_signal(&tp->m_thread_cond);
            break;
        }

        do {
            auto task = info.task;
            auto destructor = info.destructor;
            info = task->Run();
            if (destructor) {
                destructor->Process(task);
            }
        } while (info.task != nullptr);
    }

    return nullptr;
}

void ThreadPool::DoAddTask(const ThreadTaskInfo& info) {
    m_queue.Push(info);
}

bool ThreadPool::AddTask(const ThreadTaskInfo& info) {
    if (m_thread_num == 0) {
        return false;
    }

    if (!info.task) {
        return false;
    }

    DoAddTask(info);
    return true;
}

void ThreadPool::DoAddThread(unsigned int num) {
    pthread_t pid;
    for (unsigned int i = 0; i < num; ++i) {
        if (pthread_create(&pid, nullptr, ThreadWorker, this) == 0) {
            pthread_detach(pid);
            pthread_mutex_lock(&m_thread_lock);
            ++m_thread_num;
            pthread_mutex_unlock(&m_thread_lock);
        }
    }
}

void ThreadPool::AddThread(unsigned int num) {
    DoAddThread(num);
}

void ThreadPool::DoDelThread(unsigned int num) {
    ThreadTaskInfo dummy_info;
    for (unsigned int i = 0; i < num; ++i) {
        DoAddTask(dummy_info);
    }
}

void ThreadPool::DelThread(unsigned int num) {
    if (num > m_thread_num) {
        num = m_thread_num;
    }
    DoDelThread(num);
}

ThreadPool::ThreadPool() {
    m_thread_num = 0;
    pthread_mutex_init(&m_thread_lock, nullptr);
    pthread_cond_init(&m_thread_cond, nullptr);
}

ThreadPool::~ThreadPool() {
    DoDelThread(m_thread_num);

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
