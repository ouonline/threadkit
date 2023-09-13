#ifndef __THREADKIT_THREADPOOL_H__
#define __THREADKIT_THREADPOOL_H__

#include "queue.h"
#include "barrier.h"
#include "event.h"
#include <stdint.h>
#include <thread>
#include <vector>
#include <memory>
#include <functional>

namespace threadkit {

class ThreadTask {
public:
    virtual ~ThreadTask() {}
    /**
       returns a task that will be executed right after Run() returns,
       or nullptr to pick up the next item from task queue.
    */
    virtual std::shared_ptr<ThreadTask> Run() = 0;
};

class ThreadPool final {
public:
    ~ThreadPool() {
        Destroy();
    }

    bool Init(uint32_t thread_num = 0);
    void Destroy();

    bool AddTask(const std::shared_ptr<ThreadTask>& task);

private:
    static void ThreadFunc(Queue<std::shared_ptr<ThreadTask>>*);

private:
    Queue<std::shared_ptr<ThreadTask>> m_queue;
    std::vector<std::thread> m_thread_list;
};

class FixedThreadPool final {
public:
    ~FixedThreadPool() {
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
    Event m_sync_event;
    std::vector<std::thread> m_thread_list;
};

}

#endif
