#ifndef __THREADKIT_THREADPOOL_H__
#define __THREADKIT_THREADPOOL_H__

#include "scheduler.h"
#include <thread>
#include <vector>

namespace threadkit {

class ThreadTask : public MPSCQueue::Node {
public:
    ThreadTask() : deleter(nullptr) {}
    virtual ~ThreadTask() {}

    /**
       @brief returns a task that will be executed right after Run() returns,
       or nullptr to pick up a task from task queue.
       @param thread_idx 0 <= thread_idx < number of threads
    */
    virtual ThreadTask* Run(uint32_t thread_idx) = 0;

    void (*deleter)(ThreadTask* self);
};

class ThreadPool final {
public:
    ThreadPool() {}

    ~ThreadPool() {
        Destroy();
    }

    bool Init(uint32_t thread_num = 0);
    void Destroy();

    uint32_t GetThreadNum() const {
        return m_thread_list.size();
    }

    bool AddTask(ThreadTask* task);

    /**
       @brief assign `task` to thread `prefer_thread_idx`.
       @note `task` may be scheduled to other thread if thread `prefer_thread_idx` is busy.
    */
    bool AddTask(ThreadTask* task, uint32_t prefer_thread_idx);

private:
    static void ThreadFunc(uint32_t thread_idx, Scheduler*);

private:
    Scheduler m_sched;
    std::vector<std::thread> m_thread_list;

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    void operator=(const ThreadPool&) = delete;
    void operator=(ThreadPool&&) = delete;
};

}

#endif
