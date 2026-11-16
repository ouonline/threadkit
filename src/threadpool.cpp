#include "threadkit/threadpool.h"
#include "threadkit/cond_var.h"
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
        thread_num = max(thread::hardware_concurrency(), 2u) - 1;
    }

    if (!m_sched.Init(thread_num)) {
        return false;
    }

    m_sched.Start();

    m_thread_list.reserve(thread_num);
    for (uint32_t i = 0; i < thread_num; ++i) {
        m_thread_list.emplace_back(thread(ThreadFunc, i, &m_sched));
    }

    return true;
}

bool ThreadPool::AddTask(ThreadTask* task) {
    if (!m_sched.IsActive()) {
        return false;
    }
    m_sched.Push(task);
    return true;
}

void ThreadPool::Destroy() {
    if (!m_thread_list.empty()) {
        m_sched.Stop();
        for (auto t = m_thread_list.begin(); t != m_thread_list.end(); ++t) {
            t->join();
        }
        m_thread_list.clear();
    }
}

/* ------------------------------------------------------------------------- */

bool StaticThreadPool::Init(uint32_t thread_num) {
    if (!m_thread_list.empty()) {
        return true;
    }

    if (thread_num == 0) {
        thread_num = max(thread::hardware_concurrency(), 2u) - 1;
    }

    m_stop.store(false, memory_order_relaxed);
    m_start_barrier.ResetMaxCount(thread_num + 1);

    m_thread_list.reserve(thread_num);
    for (uint32_t i = 0; i < thread_num; ++i) {
        m_thread_list.emplace_back(thread(&StaticThreadPool::ThreadFunc, this, i));
    }

    return true;
}

void StaticThreadPool::Destroy() {
    if (!m_thread_list.empty()) {
        m_stop.store(true, memory_order_release);
        m_start_barrier.Wait();
        for (uint32_t i = 0; i < m_thread_list.size(); ++i) {
            m_thread_list[i].join();
        }
        m_func = nullptr;
        m_thread_list.clear();
    }
}

void StaticThreadPool::ThreadFunc(uint32_t thread_idx) {
    while (true) {
        m_start_barrier.Wait();
        if (m_stop.load(memory_order_acquire) || !m_func) {
            break;
        }
        m_func(thread_idx);
        m_nr_finished.fetch_add(1, memory_order_acq_rel);
        m_cond.NotifyOne();
    }
}

void StaticThreadPool::ParallelRunAsync(const function<void(uint32_t)>& f) {
    m_func = f;
    if (f) {
        m_nr_finished.store(0, memory_order_release);
        m_start_barrier.Wait();
    }
}

void StaticThreadPool::ParallelRun(const function<void(uint32_t)>& f) {
    if (f) {
        m_func = f;
        m_nr_finished.store(0, memory_order_release);
        m_start_barrier.Wait();
        m_cond.Wait([&]() -> bool {
            return (m_nr_finished.load(memory_order_acquire) >=
                    m_thread_list.size());
        });

        m_func = nullptr;
    }
}

}
