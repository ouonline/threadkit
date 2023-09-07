#ifndef __THREADKIT_THREADPOOL_H__
#define __THREADKIT_THREADPOOL_H__

#include "queue.h"
#include <stdint.h>
#include <thread>
#include <memory>

/* ------------------------------------------------------------------------- */

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

class JoinableThreadTask : public ThreadTask {
public:
    std::shared_ptr<ThreadTask> Run() override final;
    void Join();

protected:
    JoinableThreadTask() {}
    virtual ~JoinableThreadTask() {}

    virtual bool IsFinished() const = 0;
    virtual std::shared_ptr<ThreadTask> Process() = 0;

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;

private:
    JoinableThreadTask(const JoinableThreadTask&) = delete;
    JoinableThreadTask(JoinableThreadTask&&) = delete;
    void operator=(const JoinableThreadTask&) = delete;
    void operator=(JoinableThreadTask&&) = delete;
};

/* ------------------------------------------------------------------------- */

class ThreadPool final {
public:
    ThreadPool() {}
    ~ThreadPool();

    /**
       Only queue index 0 is available if `share_task_queue` is true,
       otherwise each thread has its own task queue.
    */
    bool Init(uint32_t thread_num = 0, bool share_task_queue = true);
    bool AddTask(const std::shared_ptr<ThreadTask>& task, uint32_t queue_idx = 0);

private:
    typedef Queue<std::shared_ptr<ThreadTask>> TaskQueue;

private:
    static void ThreadFunc(TaskQueue*);

private:
    uint32_t m_thread_num = 0;
    uint32_t m_queue_num = 0;
    std::thread* m_thread_list = nullptr;
    TaskQueue* m_queue_list = nullptr;

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
};

}

#endif
