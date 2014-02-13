#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

static void print(void* arg)
{
    printf("%s\n", (const char*)arg);
}

int main(void)
{
    const char* str = "Hello, world!";
    struct thread_pool* tp;

    tp = thread_pool_init(5);
    if (!tp)
        return 0;

    thread_pool_add_task(tp, (void*)str, print);

    thread_pool_destroy(tp);

    return 0;
}
