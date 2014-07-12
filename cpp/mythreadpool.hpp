#ifndef __MYTHREADPOOL_HPP__
#define __MYTHREADPOOL_HPP__

#include <queue>
#include <vector>
#include <memory>
#include <pthread.h>

/* ------------------------------------------------------------------------- */

namespace myutils {

class MyThreadTask {

    public:

        virtual void run() = 0;
};

struct MyThreadTaskQueue {

    MyThreadTaskQueue()
    {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~MyThreadTaskQueue()
    {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::queue<std::shared_ptr<MyThreadTask>> tasklist;
};

/* ------------------------------------------------------------------------- */

class MyThreadPool {

    public:

        MyThreadPool(int num = 0);
        virtual ~MyThreadPool();

        bool addTask(std::shared_ptr<MyThreadTask> t);

        unsigned int threadNum() const { return m_pidlist.size(); }
        unsigned int pendingTaskNum() const { return m_queue.tasklist.size(); }

    private:

        void doAddTask(std::shared_ptr<MyThreadTask> t);

    private:

        bool m_valid;
        MyThreadTaskQueue m_queue;
        std::vector<pthread_t> m_pidlist;
};

}

#endif
