#include "threadkit/threadpool.h"
#include "threadkit/event_count.h"
#include <atomic>
#include <algorithm>
using namespace std;

namespace threadkit {

void ThreadPool::ThreadFunc(uint32_t thread_idx, MPMCBlockingQueue<shared_ptr<ThreadTask>>* q) {
    while (true) {
        auto task = q->Pop();
        if (!task) {
            break;
        }
        do {
            task = task->Run(thread_idx);
        } while (task);
    }
}

bool ThreadPool::Init(uint32_t thread_num) {
    if (thread_num == 0) {
        thread_num = std::max(std::thread::hardware_concurrency(), 2u) - 1;
    }

    m_thread_list.reserve(thread_num);
    for (uint32_t i = 0; i < thread_num; ++i) {
        m_thread_list.emplace_back(std::thread(ThreadFunc, i, &m_queue));
    }

    return true;
}

bool ThreadPool::AddTask(const shared_ptr<ThreadTask>& task) {
    if (!task) {
        return false;
    }

    m_queue.Push(task);
    return true;
}

void ThreadPool::Destroy() {
    if (!m_thread_list.empty()) {
        shared_ptr<ThreadTask> dummy_task;
        for (uint32_t i = 0; i < m_thread_list.size(); ++i) {
            m_queue.Push(dummy_task);
        }
        for (uint32_t i = 0; i < m_thread_list.size(); ++i) {
            m_thread_list[i].join();
        }
        m_thread_list.clear();
    }
}

/* ------------------------------------------------------------------------- */

bool FixedThreadPool::Init(uint32_t thread_num) {
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

void FixedThreadPool::Destroy() {
    if (!m_thread_list.empty()) {
        m_func = nullptr;
        m_start_barrier.Wait();
        for (uint32_t i = 0; i < m_thread_list.size(); ++i) {
            m_thread_list[i].join();
        }
        m_thread_list.clear();
    }
}

void FixedThreadPool::ThreadFunc(uint32_t thread_idx, Barrier* barrier, const function<void(uint32_t)>* func) {
    while (true) {
        barrier->Wait();
        if (!(*func)) {
            break;
        }
        (*func)(thread_idx);
    }
}

void FixedThreadPool::ParallelRunAsync(const function<void(uint32_t)>& f) {
    if (f) {
        m_func = f;
        m_start_barrier.Wait();
    }
}

void FixedThreadPool::ParallelRun(const function<void(uint32_t)>& f) {
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
    }
}

}
