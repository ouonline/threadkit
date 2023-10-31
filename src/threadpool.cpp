#include "threadkit/threadpool.h"
#include "threadkit/event_count.h"
#include <atomic>
using namespace std;

namespace threadkit {

void ThreadPool::ThreadFunc(uint32_t thread_idx, Scheduler* sched) {
    while (true) {
        auto node = sched->Pop(thread_idx);
        if (!node) {
            return;
        }

        auto task = static_cast<ThreadTask*>(node);

        do {
            auto prev = task;
            task = task->Run(thread_idx);
            if (prev != task) {
                if (prev->deleter) {
                    prev->deleter(prev);
                }
            }
        } while (task);
    }
}

bool ThreadPool::Init(uint32_t thread_num) {
    if (!m_thread_list.empty()) {
        return true;
    }

    if (thread_num == 0) {
        thread_num = std::max(std::thread::hardware_concurrency(), 2u) - 1;
    }

    if (!m_sched.Init(thread_num)) {
        return false;
    }

    m_thread_list.reserve(thread_num);
    for (uint32_t i = 0; i < thread_num; ++i) {
        m_thread_list.emplace_back(std::thread(ThreadFunc, i, &m_sched));
    }

    return true;
}

bool ThreadPool::AddTask(ThreadTask* task) {
    if (!task) {
        return false;
    }
    m_sched.Push(task);
    return true;
}

bool ThreadPool::AddTask(ThreadTask* task, uint32_t prefer_thread_idx) {
    if (!task) {
        return false;
    }
    m_sched.Push(task, prefer_thread_idx);
    return true;
}

void ThreadPool::Destroy() {
    if (!m_thread_list.empty()) {
        for (uint32_t i = 0; i < m_thread_list.size(); ++i) {
            m_sched.PushDummy(i);
        }
        for (auto t = m_thread_list.begin(); t != m_thread_list.end(); ++t) {
            t->join();
        }
        m_thread_list.clear();

        m_sched.Destroy();
    }
}

}
