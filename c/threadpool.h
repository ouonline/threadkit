#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include "../../kernel-utils/list.h"

struct thread_task_queue {
    unsigned int num;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct list_node tasklist;
};

struct threadpool {
    unsigned int thread_num;
    struct thread_task_queue queue;

    pthread_cond_t thread_list_cond;
    pthread_mutex_t thread_list_lock;
    struct list_node thread_list;
};

/* ------------------------------------------------------------------------- */

struct threadpool* threadpool_init(unsigned int thread_num);
void threadpool_destroy(struct threadpool*);

static inline unsigned int threadpool_size(struct threadpool* tp)
{
    return tp->thread_num;
}

unsigned int threadpool_task_num(struct threadpool* tp);

int threadpool_add_task(struct threadpool*, void* arg, void (*func)(void*),
                        void (*destructor)(void*));

void threadpool_add_thread(struct threadpool*, unsigned int num);
void threadpool_del_thread(struct threadpool*, unsigned int num);

#endif
