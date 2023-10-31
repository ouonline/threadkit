#include "threadkit/utils.h"
#include "threadkit/scheduler.h"
#include <cstdlib>
#include <new>
using namespace std;

namespace threadkit {

void Scheduler::DoMove(Scheduler&& s) {
    m_push_idx = s.m_push_idx;
    m_num = s.m_num;
    m_info_list = s.m_info_list;

    s.m_push_idx = 0;
    s.m_num = 0;
    s.m_info_list = nullptr;
}

Scheduler::Scheduler(Scheduler&& s) {
    if (&s != this) {
        DoMove(std::move(s));
    }
}

void Scheduler::operator=(Scheduler&& s) {
    if (&s != this) {
        Destroy();
        DoMove(std::move(s));
    }
}

bool Scheduler::Init(uint32_t num) {
    m_info_list = (Info*)malloc(sizeof(Info) * num);
    if (!m_info_list) {
        return false;
    }

    for (uint32_t i = 0; i < num; ++i) {
        new (m_info_list + i) Info();
    }
    m_num = num;

    return true;
}

void Scheduler::Destroy() {
    if (m_info_list) {
        for (uint32_t i = 0; i < m_num; ++i) {
            auto maybe_empty = m_info_list[i].queue.Push(&m_info_list[i].dummy);
            if (maybe_empty) {
                m_info_list[i].queue_size.fetch_add(1, std::memory_order_acq_rel);
                m_info_list[i].cond.NotifyOne();
            }
        }
        for (uint32_t i = 0; i < m_num; ++i) {
            m_info_list[i].~Info();
        }
        free(m_info_list);
        m_info_list = nullptr;
    }
}

void Scheduler::Push(MPSCQueue::Node* node, uint32_t idx) {
    auto info = &m_info_list[idx];
    auto maybe_empty = info->queue.Push(node);
    info->queue_size.fetch_add(1, std::memory_order_acq_rel);
    if (maybe_empty) {
        info->cond.NotifyOne();
    }
}

bool Scheduler::AskForReqInRange(uint32_t begin, uint32_t end, uint32_t curr) {
    for (uint32_t i = begin; i < end; ++i) {
        auto info = &m_info_list[i];
        if (info->queue_size.load(std::memory_order_relaxed) < 2) {
            continue;
        }

        uint32_t expected = UINT32_MAX;
        if (info->req_idx.compare_exchange_strong(expected, curr, std::memory_order_acq_rel)) {
            return true;
        }
    }

    return false;
}

// TODO optimize: use bitmap to accerarate lookup or choose one randomly
void Scheduler::AskForReq(uint32_t curr_idx) {
    if (!AskForReqInRange(0, curr_idx, curr_idx)) {
        AskForReqInRange(curr_idx + 1, m_num, curr_idx);
    }
}

MPSCQueue::Node* Scheduler::Pop(uint32_t idx) {
    auto info = &m_info_list[idx];
    auto queue = &info->queue;
    auto cond = &info->cond;

    MPSCQueue::Node* node;
    bool is_empty = true;

retry:
    node = queue->Pop(&is_empty);
    if (is_empty) {
        auto key = cond->PrepareWait();
        node = queue->Pop(&is_empty);
        if (is_empty) {
            AskForReq(idx);
            cond->CommitWait(key);
            goto retry;
        } else if (!node) { // inserting
            cond->CancelWait();
            goto retry;
        }
    } else if (!node) { // inserting
        goto retry;
    }

    auto nr_req = info->queue_size.fetch_sub(1, std::memory_order_acq_rel) / 2;
    if (nr_req > 0) {
        auto steal_idx = info->req_idx.load(std::memory_order_relaxed);
        if (steal_idx != UINT32_MAX) {
            auto steal_info = &m_info_list[steal_idx];

            uint32_t nr_req_stolen = 0;
            for (; nr_req_stolen < nr_req; ++nr_req_stolen) {
                MPSCQueue::Node* req = queue->Pop(&is_empty);
                if (unlikely(req == &info->dummy)) {
                    queue->Push(req); // last dummy node to terminate the thread
                    break;
                }
                steal_info->queue.Push(req);
            }

            if (likely(nr_req_stolen > 0)) {
                steal_info->queue_size.fetch_add(nr_req_stolen, std::memory_order_acq_rel);
                steal_info->cond.NotifyAll();
                info->queue_size.fetch_sub(nr_req_stolen, std::memory_order_relaxed);
            }

            info->req_idx.store(UINT32_MAX, std::memory_order_release);
        }
    }

    if (node == &info->dummy) {
        if (info->queue_size.load(std::memory_order_relaxed) == 0) {
            return nullptr;
        }

        // push the dummy node to the end of queue to make sure that all tasks will be processed
        info->queue.Push(node);
        info->queue_size.fetch_add(1, std::memory_order_release);
        goto retry;
    }

    return node;
}

}
