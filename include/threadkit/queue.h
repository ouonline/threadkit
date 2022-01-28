#ifndef __THREADKIT_QUEUE_H__
#define __THREADKIT_QUEUE_H__

#include <pthread.h>
#include <list>

namespace outils {

template <typename T>
class Queue {

public:
    Queue() {
        pthread_mutex_init(&m_mutex, nullptr);
        pthread_cond_init(&m_cond, nullptr);
    }

    ~Queue() {
        pthread_cond_destroy(&m_cond);
        pthread_mutex_destroy(&m_mutex);
    }

    void Push(const T& item) {
        pthread_mutex_lock(&m_mutex);
        m_items.push_back(item);
        pthread_mutex_unlock(&m_mutex);
        pthread_cond_signal(&m_cond);
    }

    void Push(T&& item) {
        pthread_mutex_lock(&m_mutex);
        m_items.push_back(item);
        pthread_mutex_unlock(&m_mutex);
        pthread_cond_signal(&m_cond);
    }

    T Pop() {
        pthread_mutex_lock(&m_mutex);
        while (m_items.empty()) {
            pthread_cond_wait(&m_cond, &m_mutex);
        }
        auto item = m_items.front();
        m_items.pop_front();
        pthread_mutex_unlock(&m_mutex);
        return item;
    }

    size_t Size() const {
        return m_items.size();
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    std::list<T> m_items;
};

}

#endif
