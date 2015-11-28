#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <queue>
#include <vector>
#include <memory>
#include <unordered_set>
#include <pthread.h>

/* ------------------------------------------------------------------------- */

namespace utils {

class ThreadTask {

    public:

        virtual void Run() = 0;
        virtual ~ThreadTask() {}
};

struct ThreadTaskQueue {

    ThreadTaskQueue()
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~ThreadTaskQueue()
    {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::queue<std::shared_ptr<ThreadTask>> tasklist;
};

/* ------------------------------------------------------------------------- */

class ThreadPool {

    public:

        ThreadPool(unsigned int num = 0);
        virtual ~ThreadPool();

        bool AddTask(const std::shared_ptr<ThreadTask>&);

        unsigned int ThreadNum() const { return m_thread_list.size(); }
        unsigned int TaskNum() const { return m_queue.tasklist.size(); }

        void AddThread(unsigned int num = 1);
        void DelThread(unsigned int num = 1);

    private:

        void DoAddThread();
        void DoDelThread();
        void DoAddTask(const std::shared_ptr<ThreadTask>&);

    private:

        static void* ThreadWorker(void*);

    private:

        ThreadTaskQueue m_queue;

        pthread_cond_t m_thread_cond;
        pthread_mutex_t m_thread_lock;
        std::unordered_set<pthread_t> m_thread_list;
};

}

#endif
