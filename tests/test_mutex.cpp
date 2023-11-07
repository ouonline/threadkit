#include "threadkit/threadpool.h"
#include "threadkit/mutex.h"
using namespace threadkit;

#undef NDEBUG
#include <assert.h>

#include <chrono>
#include <mutex>
#include <iostream>
using namespace std;

class TimeRecord final {
public:
    TimeRecord(uint64_t* ms) : m_ms(ms) {
        m_begin_ts = std::chrono::system_clock::now();
    }
    ~TimeRecord() {
        auto end = std::chrono::system_clock::now();
        *m_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - m_begin_ts).count();
    }
private:
    std::chrono::time_point<std::chrono::system_clock> m_begin_ts;
    uint64_t* m_ms;
};

class TestTask final : public ThreadTask {
public:
    TestTask(uint64_t* counter, Mutex* lck) : m_counter(counter), m_lock(lck) {}
    ThreadTask* Run(uint32_t) override {
        return nullptr;
    }

private:
    uint64_t* m_counter;
    Mutex* m_lock;
};

#define N 5
#define M 10000

class StdMutex final {
public:
    void Lock() {
        m_mtx.lock();
    }
    void Unlock() {
        m_mtx.unlock();
    }
private:
    std::mutex m_mtx;
};

template <typename T>
static void TestFunc(uint64_t* time_cost) {
    StaticThreadPool tp;
    assert(tp.Init(N));

    T mtx;
    uint64_t counter = 0;
    {
        TimeRecord rec(time_cost);
        tp.ParallelRun([&mtx, &counter](uint32_t idx) -> void {
            for (uint32_t i = 0; i < M; ++i) {
                mtx.Lock();
                ++counter;
                mtx.Unlock();
            }
        });
    }
    assert(counter == N * M);
}

#define NR_LOOP 100

template <typename T>
static void TestMulti(uint64_t* avg_time_cost) {
    uint64_t sum = 0;
    for (uint32_t i = 0; i < NR_LOOP; ++i) {
        uint64_t time_cost = 0;
        TestFunc<T>(&time_cost);
        sum += time_cost;
    }
    *avg_time_cost = sum / NR_LOOP;
}

int main(void) {
    uint64_t mutex_avg_time = 0, std_mutex_avg_time = 0;
    TestMulti<Mutex>(&mutex_avg_time);
    TestMulti<StdMutex>(&std_mutex_avg_time);
    cout << "mutex avg time [" << mutex_avg_time << "], std mutex avg time [" << std_mutex_avg_time << "]" << endl;
    return 0;
}
