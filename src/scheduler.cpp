#include "threadkit/utils.h"
#include "threadkit/scheduler.h"
#include <cstdlib>
#include <new>
using namespace std;

namespace threadkit {

void Scheduler::DoMove(Scheduler&& s) {
    m_push_idx.store(s.m_push_idx.load(std::memory_order_relaxed), std::memory_order_relaxed);
    m_num = s.m_num;
    m_info_list = s.m_info_list;

    s.m_push_idx.store(0, std::memory_order_relaxed);
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
    m_active = true;

    return true;
}

void Scheduler::Destroy() {
    if (m_info_list) {
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
    if (!node) {
        if (!is_empty) {
            goto retry;
        }
        if (!m_active) {
            return nullptr;
        }

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
    }

    auto nr_req = info->queue_size.fetch_sub(1, std::memory_order_acq_rel) / 2;
    if (nr_req > 0) {
        auto steal_idx = info->req_idx.load(std::memory_order_relaxed);
        if (steal_idx != UINT32_MAX) {
            auto steal_info = &m_info_list[steal_idx];
            for (uint32_t i = 0; i < nr_req; ++i) {
                steal_info->queue.Push(queue->Pop(&is_empty));
            }

            steal_info->queue_size.fetch_add(nr_req, std::memory_order_acq_rel);
            steal_info->cond.NotifyAll();
            info->queue_size.fetch_sub(nr_req, std::memory_order_relaxed);
            info->req_idx.store(UINT32_MAX, std::memory_order_release);
        }
    }

    return node;
}

void Scheduler::Stop() {
    m_active = false;
    for (uint32_t i = 0; i < m_num; ++i) {
        m_info_list[i].cond.NotifyAll();
    }
}

}
