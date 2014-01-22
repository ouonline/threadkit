#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <queue>
#include <vector>
#include <memory>

using std::queue;
using std::vector;

#include <pthread.h>

/* ------------------------------------------------------------------------- */

class ThreadTaskEntity {

    public:

        virtual void run() = 0;
};

struct ThreadTaskQueue {

    ThreadTaskQueue()
    {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~ThreadTaskQueue()
    {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    queue<std::shared_ptr<ThreadTaskEntity>> tasklist;
};

/* ------------------------------------------------------------------------- */

class ThreadPool {

    public:

        ThreadPool(int num = 0);
        virtual ~ThreadPool();

        bool addTask(const std::shared_ptr<ThreadTaskEntity>& t);

    private:

        void doAddTask(const std::shared_ptr<ThreadTaskEntity>& t);

    private:

        bool m_valid;
        ThreadTaskQueue m_queue;
        vector<pthread_t> m_pidlist;
};

#endif
