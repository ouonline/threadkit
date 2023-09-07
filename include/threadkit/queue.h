#ifndef __THREADKIT_QUEUE_H__
#define __THREADKIT_QUEUE_H__

#include <mutex>
#include <condition_variable>
#include <list>

namespace threadkit {

template <typename T>
class Queue final {
public:
    Queue() {}

    void Push(const T& item) {
        m_mutex.lock();
        m_items.push_back(item);
        m_mutex.unlock();
        m_cond.notify_one();
    }

    void Push(T&& item) {
        m_mutex.lock();
        m_items.emplace_back(std::move(item));
        m_mutex.unlock();
        m_cond.notify_one();
    }

    T Pop() {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_cond.wait(lck, [this]() -> bool {
            return (!m_items.empty());
        });
        auto item = m_items.front();
        m_items.pop_front();
        return item;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::list<T> m_items;

private:
    Queue(const Queue&) = delete;
    void operator=(const Queue&) = delete;
    Queue(Queue&&) = delete;
    Queue& operator=(Queue&&) = delete;
};

}

#endif
