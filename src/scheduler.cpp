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
    if (maybe_empty) {
        info->cond.NotifyOne();
    }
}

void Scheduler::Push(MPSCQueue::Node* node) {
    auto idx = m_push_idx.fetch_add(1, std::memory_order_acquire) % m_num;
    Push(node, idx);
}

MPSCQueue::Node* Scheduler::AskForReqInRange(uint32_t begin, uint32_t end) {
    bool is_empty;
    for (uint32_t i = begin; i < end; ++i) {
        auto info = &m_info_list[i];
        info->pop_lock.Lock();
        auto ret = info->queue.Pop(&is_empty);
        info->pop_lock.Unlock();
        if (ret) {
            return ret;
        }
    }

    return nullptr;
}

// TODO optimize: use bitmap to accerarate lookup or choose one randomly
MPSCQueue::Node* Scheduler::AskForReq(uint32_t curr_idx) {
    auto ret = AskForReqInRange(0, curr_idx);
    if (!ret) {
        ret = AskForReqInRange(curr_idx + 1, m_num);
    }
    return ret;
}

MPSCQueue::Node* Scheduler::Pop(uint32_t idx) {
    auto info = &m_info_list[idx];
    auto queue = &info->queue;
    auto cond = &info->cond;

    MPSCQueue::Node* node;
    bool is_empty = true;

retry:
    info->pop_lock.Lock();
    do {
        node = queue->Pop(&is_empty);
    } while (!node && !is_empty);
    info->pop_lock.Unlock();

    // is empty
    if (!node) {
        if (!m_active) {
            return nullptr;
        }

        auto key = cond->PrepareWait();
        node = AskForReq(idx);
        if (!node) {
            cond->CommitWait(key);
            goto retry;
        }
        cond->CancelWait();
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
