#ifndef __THREADKIT_THREADPOOL_H__
#define __THREADKIT_THREADPOOL_H__

#include "queue.h"
#include <memory>

/* ------------------------------------------------------------------------- */

namespace outils {

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
    JoinableThreadTask();
    virtual ~JoinableThreadTask();
    std::shared_ptr<ThreadTask> Run() override final;
    void Join();

protected:
    virtual std::shared_ptr<ThreadTask> Process() = 0;
    virtual bool IsFinished() const = 0;

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

/* ------------------------------------------------------------------------- */

class ThreadPool final {

public:
    ThreadPool();
    ~ThreadPool();

    void AddTask(const std::shared_ptr<ThreadTask>&);

    unsigned int ThreadNum() const { return m_thread_num; }
    unsigned int PendingTaskNum() const { return m_queue.Size(); }

    void AddThread(unsigned int num);
    void DelThread(unsigned int num);

private:
    void DoAddTask(const std::shared_ptr<ThreadTask>&);

private:
    static void* ThreadFunc(void*);

private:
    unsigned int m_thread_num;
    pthread_mutex_t m_thread_lock;
    pthread_cond_t m_thread_cond;
    Queue<std::shared_ptr<ThreadTask>> m_queue;

private:
    ThreadPool(const ThreadPool&);
    ThreadPool& operator=(const ThreadPool&);
};

}

#endif
