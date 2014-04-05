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
    struct threadpool* tp;

    tp = threadpool_init(5);
    if (!tp)
        return 0;

    threadpool_add_task(tp, (void*)str, print);

    threadpool_destroy(tp);

    return 0;
}
