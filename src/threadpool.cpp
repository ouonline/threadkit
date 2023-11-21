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

    m_sched.Start();

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

        m_sched.Destroy();
    }
}

/* ------------------------------------------------------------------------- */

bool StaticThreadPool::Init(uint32_t thread_num) {
    if (thread_num == 0) {
        thread_num = std::max(std::thread::hardware_concurrency(), 2u) - 1;
    }

    m_start_barrier.ResetMaxCount(thread_num + 1);

    m_thread_list.reserve(thread_num);
    for (uint32_t i = 0; i < thread_num; ++i) {
        m_thread_list.emplace_back(std::thread(ThreadFunc, i, &m_start_barrier, &m_func));
    }

    return true;
}

void StaticThreadPool::Destroy() {
    if (!m_thread_list.empty()) {
        m_func = nullptr;
        m_start_barrier.Wait();
        for (uint32_t i = 0; i < m_thread_list.size(); ++i) {
            m_thread_list[i].join();
        }
        m_thread_list.clear();
    }
}

void StaticThreadPool::ThreadFunc(uint32_t thread_idx, Barrier* barrier, const function<void(uint32_t)>* func) {
    while (true) {
        barrier->Wait();
        if (!(*func)) {
            break;
        }
        (*func)(thread_idx);
    }
}

void StaticThreadPool::ParallelRunAsync(const function<void(uint32_t)>& f) {
    m_func = f;
    if (f) {
        m_start_barrier.Wait();
    }
}

void StaticThreadPool::ParallelRun(const function<void(uint32_t)>& f) {
    if (f) {
        EventCount cond;
        atomic<uint32_t> nr_finished(0);
        auto nr_thread = m_thread_list.size();

        m_func = [&nr_finished, nr_thread, &cond, &f](uint32_t thread_idx) -> void {
            f(thread_idx);
            auto count = nr_finished.fetch_add(1, std::memory_order_acq_rel) + 1;
            if (count >= nr_thread) {
                cond.NotifyOne();
            }
        };
        m_start_barrier.Wait();

        cond.Wait([nr_thread, &nr_finished]() -> bool {
            return (nr_finished.load(std::memory_order_acquire) >= nr_thread);
        });

        m_func = nullptr;
    }
}

}
