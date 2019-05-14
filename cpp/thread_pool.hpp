#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <queue>
#include <pthread.h>

/* ------------------------------------------------------------------------- */

namespace utils {

class ThreadTask {

public:
    ThreadTask();
    virtual ~ThreadTask();
    void Exec();
    void Join();

protected:
    virtual void Run() = 0;
    virtual bool IsFinished() const = 0;

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

struct ThreadTaskQueue {

    ThreadTaskQueue() {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~ThreadTaskQueue() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::queue<ThreadTask*> tasklist;
};

/* ------------------------------------------------------------------------- */

class ThreadPool {

public:
    ThreadPool();
    virtual ~ThreadPool();

    bool AddTask(ThreadTask*);

    unsigned int ThreadNum() const { return m_thread_num; }
    unsigned int PendingTaskNum() const { return m_queue.tasklist.size(); }

    void AddThread(unsigned int num);
    void DelThread(unsigned int num);

private:
    void DoAddThread();
    void DoDelThread();
    void DoAddTask(ThreadTask*);

private:
    static void* ThreadWorker(void*);

private:
    ThreadTaskQueue m_queue;
    unsigned int m_thread_num;
    pthread_cond_t m_thread_cond;
    pthread_mutex_t m_thread_lock;
};

}

#endif
