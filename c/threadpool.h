#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include "../../kernel-utils/list.h"

struct thread_task_queue {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct list_node tasklist;
};

struct thread_pool {
    int thread_num;
    struct thread_task_queue queue;
    pthread_t pidlist[0];
};

struct thread_pool* thread_pool_init(int thread_num);
void thread_pool_destroy(struct thread_pool*);

int thread_pool_add_task(struct thread_pool*, void* arg, void (*func)(void*));

#endif
