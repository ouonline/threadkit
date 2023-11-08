#ifndef __THREADKIT_SCHEDULER_H__
#define __THREADKIT_SCHEDULER_H__

#include "mpsc_queue.h"
#include "event_count.h"
#include "mutex.h"

namespace threadkit {

class Scheduler final {
public:
    Scheduler() : m_active(false), m_push_idx(0), m_num(0), m_info_list(nullptr) {}

    Scheduler(Scheduler&&);
    void operator=(Scheduler&&);

    ~Scheduler() {
        Destroy();
    }

    bool Init(uint32_t num);
    void Destroy();

    void Push(MPSCQueue::Node* node);

    // blocks if queue is empty. returns nullptr for dummy node.
    MPSCQueue::Node* Pop(uint32_t idx);

    void Stop();

    bool IsActive() const {
        return m_active;
    }

private:
    void DoMove(Scheduler&&);
    MPSCQueue::Node* AskForReqInRange(uint32_t begin, uint32_t end);
    MPSCQueue::Node* AskForReq(uint32_t curr);

    /**
       @brief assign `node` to `prefer_idx`.
       @note `node` may be scheduled to other consumers if `prefer_idx` is busy.
    */
    void Push(MPSCQueue::Node* node, uint32_t prefer_idx);

private:
    struct Info final {
        MPSCQueue queue;
        EventCount cond;
        Mutex pop_lock;
    };

private:
    bool m_active;
    std::atomic<uint32_t> m_push_idx;
    uint32_t m_num;
    Info* m_info_list;

private:
    Scheduler(const Scheduler&) = delete;
    void operator=(const Scheduler&) = delete;
};

}

#endif
