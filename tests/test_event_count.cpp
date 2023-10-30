#include "threadkit/event_count.h"
#include <iostream>
using namespace std;
using namespace threadkit;

#undef NDEBUG
#include <assert.h>

#ifdef _MSC_VER
#include <windows.h>
static void SleepSec(int sec) {
    Sleep(sec * 1000);
}
#else
#include <unistd.h>
static void SleepSec(int sec) {
    sleep(sec);
}
#endif

#include <thread>
#include <vector>

static void TestNotifyOne() {
    int value = 0;
    EventCount ec;

    thread t1([&value, &ec]() -> void {
        ec.Wait([&value]() -> bool {
            cout << "thread1: test value == " << value << endl;
            return (value == 5);
        });
    });

    cout << "thread1 created." << endl;
    SleepSec(3);

    thread t2([&value, &ec]() -> void {
        value = 5;
        cout << "thread2: set value = 5" << endl;
        ec.NotifyOne();
    });

    t1.join();
    t2.join();

    assert(value == 5);
}

static void TestNotifyAll() {
    int value = 0;
    EventCount ec;

    vector<thread> thread_list;
    for (uint32_t i = 0; i < 5; ++i) {
        thread_list.emplace_back([&value, &ec]() -> void {
            ec.Wait([&value]() -> bool {
                cout << "thread1: test value == " << value << endl;
                return (value == 5);
            });
        });
    }

    cout << thread_list.size() << " threads created. wait for 3s" << endl;
    SleepSec(3);

    thread t2([&value, &ec]() -> void {
        value = 5;
        cout << "thread2: set value = 5" << endl;
        ec.NotifyAll();
    });

    for (auto t = thread_list.begin(); t != thread_list.end(); ++t) {
        t->join();
    }
    t2.join();

    assert(value == 5);
}

int main(void) {
    TestNotifyOne();
    TestNotifyAll();
    return 0;
}
