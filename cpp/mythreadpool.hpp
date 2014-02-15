#ifndef __MYTHREADPOOL_HPP__
#define __MYTHREADPOOL_HPP__

#include <queue>
#include <vector>
#include <memory>
#include <pthread.h>

using std::queue;
using std::vector;
using std::shared_ptr;

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
    queue<shared_ptr<MyThreadTask>> tasklist;
};

/* ------------------------------------------------------------------------- */

class MyThreadPool {

    public:

        MyThreadPool(int num = 0);
        virtual ~MyThreadPool();

        bool addTask(const shared_ptr<MyThreadTask>& t);

    private:

        void doAddTask(const shared_ptr<MyThreadTask>& t);

    private:

        bool m_valid;
        MyThreadTaskQueue m_queue;
        vector<pthread_t> m_pidlist;
};

}

#endif
