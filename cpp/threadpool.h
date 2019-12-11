#ifndef __UTILS_THREADPOOL_H__
#define __UTILS_THREADPOOL_H__

#include "queue.h"

/* ------------------------------------------------------------------------- */

namespace utils {

struct ThreadTaskInfo;

class ThreadTask {

public:
    virtual ~ThreadTask() {}
    /*
     * returns <task, destructor> to be executed right after Run() returns,
     * or <nullptr, destructor> to pick up the next item from task queue.
     */
    virtual ThreadTaskInfo Run() = 0;
};

class ThreadTaskDestructor {

public:
    virtual ~ThreadTaskDestructor() {}
    virtual void Process(ThreadTask*) = 0;
};

class JoinableThreadTask : public ThreadTask {

public:
    JoinableThreadTask();
    virtual ~JoinableThreadTask();
    ThreadTaskInfo Run() override final;
    void Join();

protected:
    virtual ThreadTaskInfo Process() = 0;
    virtual bool IsFinished() const = 0;

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

struct ThreadTaskInfo {
    ThreadTaskInfo(ThreadTask* t = nullptr, ThreadTaskDestructor* d = nullptr)
        : task(t), destructor(d) {}
    ThreadTask* task;
    ThreadTaskDestructor* destructor;
};

/* ------------------------------------------------------------------------- */

class ThreadPool final {

public:
    ThreadPool();
    virtual ~ThreadPool();

    void AddTask(const ThreadTaskInfo&);

    template <template <typename...> class ContainerType>
    void BatchAddTask(const ContainerType<ThreadTaskInfo>& tasks) {
        m_queue.BatchPush(tasks, [] (const ThreadTaskInfo& info) -> bool {
            return (info.task != nullptr);
        });
    }

    unsigned int ThreadNum() const { return m_thread_num; }
    unsigned int PendingTaskNum() const { return m_queue.Size(); }

    void AddThread(unsigned int num);
    void DelThread(unsigned int num);

private:
    void DoAddTask(const ThreadTaskInfo&);

private:
    static void* ThreadWorker(void*);

private:
    unsigned int m_thread_num;
    pthread_mutex_t m_thread_lock;
    pthread_cond_t m_thread_cond;
    Queue<ThreadTaskInfo> m_queue;
};

}

#endif
