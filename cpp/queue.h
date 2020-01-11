#ifndef __UTILS_QUEUE_H__
#define __UTILS_QUEUE_H__

#include <pthread.h>
#include <queue>
#include <functional>

namespace utils {

template <typename T>
class Queue {

public:
    Queue() {
        m_item_count = 0;
        pthread_mutex_init(&m_mutex, nullptr);
        pthread_cond_init(&m_cond, nullptr);
    }

    ~Queue() {
        pthread_cond_destroy(&m_cond);
        pthread_mutex_destroy(&m_mutex);
    }

    void Push(const T& item) {
        pthread_mutex_lock(&m_mutex);
        m_items.push(item);
        ++m_item_count;
        pthread_mutex_unlock(&m_mutex);
        pthread_cond_signal(&m_cond);
    }

    void BatchPush(const std::function<void (const std::function<void (const T&)>&)>& generator) {
        unsigned int counter = 0;

        auto push_helper = [this, &counter] (const T& item) -> void {
            m_items.push(item);
            ++counter;
        };

        pthread_mutex_lock(&m_mutex);
        generator(push_helper);
        m_item_count += counter;
        pthread_mutex_unlock(&m_mutex);

        while (counter > 0) {
            pthread_cond_signal(&m_cond);
            --counter;
        }
    }

    T Pop() {
        pthread_mutex_lock(&m_mutex);
        while (m_items.empty()) {
            pthread_cond_wait(&m_cond, &m_mutex);
        }
        auto item = m_items.front();
        m_items.pop();
        --m_item_count;
        pthread_mutex_unlock(&m_mutex);
        return item;
    }

    size_t Size() const {
        return m_item_count;
    }

private:
    unsigned int m_item_count;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    std::queue<T> m_items;
};

}

#endif
