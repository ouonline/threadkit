#ifndef __THREADKIT_THREADPOOL_H__
#define __THREADKIT_THREADPOOL_H__

#include "barrier.h"
#include "scheduler.h"
#include <thread>
#include <vector>
#include <functional>

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

class StaticThreadPool final {
public:
    ~StaticThreadPool() {
        Destroy();
    }

    bool Init(uint32_t thread_num = 0);
    void Destroy();

    uint32_t GetThreadNum() const {
        return m_thread_list.size();
    }

    void ParallelRun(const std::function<void(uint32_t thread_idx)>&);

    /** Caller MUST make sure that last tasks are finished before starting another new call. */
    void ParallelRunAsync(const std::function<void(uint32_t thread_idx)>&);

private:
    static void ThreadFunc(uint32_t, Barrier*, const std::function<void(uint32_t)>*);

private:
    std::function<void(uint32_t)> m_func;
    Barrier m_start_barrier;
    std::vector<std::thread> m_thread_list;
};

}

#endif
