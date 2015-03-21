#ifndef __MYTHREADPOOL_HPP__
#define __MYTHREADPOOL_HPP__

#include <queue>
#include <vector>
#include <memory>
#include <unordered_set>
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

        MyThreadPool(unsigned int num = 0);
        virtual ~MyThreadPool();

        bool addTask(const std::shared_ptr<MyThreadTask>&);

        unsigned int threadNum() const { return m_thread_list.size(); }
        unsigned int taskNum() const { return m_queue.tasklist.size(); }

        void addThread(unsigned int num = 1);
        void delThread(unsigned int num = 1);

    private:

        void doAddThread();
        void doDelThread();
        void doAddTask(const std::shared_ptr<MyThreadTask>&);

    private:

        static void* thread_worker(void*);

    private:

        MyThreadTaskQueue m_queue;

        pthread_cond_t m_thread_cond;
        pthread_mutex_t m_thread_lock;
        std::unordered_set<pthread_t> m_thread_list;
};

}

#endif
