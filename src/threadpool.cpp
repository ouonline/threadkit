#include "threadkit/threadpool.h"
#include <algorithm>
using namespace std;

namespace threadkit {

shared_ptr<ThreadTask> JoinableThreadTask::Run() {
    shared_ptr<ThreadTask> next_task;
    if (!IsFinished()) {
        m_mutex.lock();
        next_task = Process();
        m_cond.notify_one();
        m_mutex.unlock();
    }
    return next_task;
}

void JoinableThreadTask::Join() {
    std::unique_lock<std::mutex> lck(m_mutex);
    m_cond.wait(lck, [this]() -> bool {
        return IsFinished();
    });
}

/* -------------------------------------------------------------------------- */

void ThreadPool::ThreadFunc(TaskQueue* q) {
    while (true) {
        auto task = q->Pop();
        if (!task) {
            break;
        }
        do {
            task = task->Run();
        } while (task);
    }
}

bool ThreadPool::Init(uint32_t thread_num, bool share_task_queue) {
    if (thread_num == 0) {
        thread_num = std::max(std::thread::hardware_concurrency(), 1u);
    }

    if (share_task_queue) {
        m_queue_list = (TaskQueue*)malloc(sizeof(TaskQueue));
        if (!m_queue_list) {
            return false;
        }
        new (m_queue_list) TaskQueue();
        m_queue_num = 1;
    } else {
        m_queue_list = (TaskQueue*)malloc(thread_num * sizeof(TaskQueue));
        if (!m_queue_list) {
            return false;
        }
        for (uint32_t i = 0; i < thread_num; ++i) {
            new (m_queue_list + i) TaskQueue();
        }
        m_queue_num = thread_num;
    }

    m_thread_list = (std::thread*)malloc(thread_num * sizeof(std::thread));
    if (!m_thread_list) {
        for (uint32_t i = 0; i < m_queue_num; ++i) {
            m_queue_list[i].~TaskQueue();
        }
        free(m_queue_list);
        return false;
    }

    for (uint32_t i = 0; i < thread_num; ++i) {
        if (share_task_queue) {
            new (m_thread_list + i) std::thread(ThreadFunc, m_queue_list);
        } else {
            new (m_thread_list + i) std::thread(ThreadFunc, m_queue_list + i);
        }
    }
    m_thread_num = thread_num;

    return true;
}

bool ThreadPool::AddTask(const shared_ptr<ThreadTask>& task, uint32_t queue_idx) {
    if (!task) {
        return false;
    }

    m_queue_list[queue_idx].Push(task);
    return true;
}

ThreadPool::~ThreadPool() {
    if (!m_thread_list) {
        return;
    }

    shared_ptr<ThreadTask> dummy_task;
    if (m_queue_num == m_thread_num) {
        for (uint32_t i = 0; i < m_queue_num; ++i) {
            m_queue_list[i].Push(dummy_task);
        }
    } else {
        for (uint32_t i = 0; i < m_thread_num; ++i) {
            m_queue_list[0].Push(dummy_task);
        }
    }

    for (uint32_t i = 0; i < m_thread_num; ++i) {
        m_thread_list[i].join();
        m_thread_list[i].~thread();
    }
    free(m_thread_list);

    for (uint32_t i = 0; i < m_queue_num; ++i) {
        m_queue_list[i].~TaskQueue();
    }
    free(m_queue_list);
}

}
