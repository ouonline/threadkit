#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include "../../kernel-utils/list.h"

struct thread_task_queue {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct list_node tasklist;
};

struct threadpool {
    unsigned int thread_num;
    struct thread_task_queue queue;
    pthread_t pidlist[0];
};

struct threadpool* threadpool_init(unsigned int thread_num);
void threadpool_destroy(struct threadpool*);

int threadpool_add_task(struct threadpool*, void* arg, void (*func)(void*));

#endif
