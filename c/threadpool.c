#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"
#include "../mm/mm.h"

/* ------------------------------------------------------------------------- */

struct thread_task {
    struct list_node node;
    void* arg;
    void (*func)(void*);
};

static inline struct thread_task* thread_task_alloc(void* arg,
                                                    void (*func)(void*))
{
    struct thread_task* t;

    t = mm_alloc(sizeof(struct thread_task));
    if (t) {
        t->arg = arg;
        t->func = func;
    }

    return t;
}

static inline void thread_task_free(struct thread_task* t)
{
    mm_free(t);
}

/* ------------------------------------------------------------------------- */

static inline void thread_task_queue_enqueue(struct thread_task_queue* q,
                                             struct thread_task* t)
{
    pthread_mutex_lock(&q->mutex);
    list_add_prev(&t->node, &q->tasklist);
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

static inline
struct thread_task* thread_task_queue_dequeue(struct thread_task_queue* q)
{
    struct list_node* t;

    pthread_mutex_lock(&q->mutex);

    while (list_empty(&q->tasklist))
        pthread_cond_wait(&q->cond, &q->mutex);

    t = list_next(&q->tasklist);
    list_del(t);

    pthread_mutex_unlock(&q->mutex);

    return list_entry(t, struct thread_task, node);
}

static inline void thread_task_queue_init(struct thread_task_queue* q)
{
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    list_init(&q->tasklist);
}

static inline void thread_task_queue_destroy(struct thread_task_queue* q)
{
    struct list_node *node, *next;

    pthread_mutex_lock(&q->mutex);
    list_for_each_safe (node, next, &q->tasklist)
        thread_task_free(list_entry(node, struct thread_task, node));
    pthread_mutex_unlock(&q->mutex);

    list_init(&q->tasklist);
    pthread_cond_destroy(&q->cond);
    pthread_mutex_destroy(&q->mutex);
}

/* ------------------------------------------------------------------------- */

static void* worker_thread(void* arg)
{
    struct thread_task_queue* q = (struct thread_task_queue*)arg;

    while (1) {
        struct thread_task* t;

        t = thread_task_queue_dequeue(q);
        if (!t->func) {
            thread_task_free(t);
            break;
        }

        t->func(t->arg);

        thread_task_free(t);
    }

    return NULL;
}

static inline int do_add_task(struct thread_pool* tp, void* arg,
                              void (*func)(void*))
{
    struct thread_task* task;

    task = thread_task_alloc(arg, func);
    if (!task)
        return -1;

    thread_task_queue_enqueue(&tp->queue, task);
    return 0;
}

int thread_pool_add_task(struct thread_pool* tp, void* arg,
                         void (*func)(void*))
{
    if (!tp || tp->thread_num == 0 || !func)
        return -1;

    return do_add_task(tp, arg, func);
}

struct thread_pool* thread_pool_init(int thread_num)
{
    int i;
    struct thread_pool* tp;

    if (thread_num <= 0)
        thread_num = sysconf(_SC_NPROCESSORS_CONF);

    tp = mm_alloc(sizeof(struct thread_pool) + sizeof(pthread_t) * thread_num);
    if (!tp)
        return NULL;

    thread_task_queue_init(&tp->queue);

    tp->thread_num = 0;

    for (i = 0; i < thread_num; ++i) {
        int err = pthread_create(&tp->pidlist[i], NULL, worker_thread,
                                 &tp->queue);
        if (!err)
            ++tp->thread_num;
    }

    if (tp->thread_num == 0) {
        mm_free(tp);
        return NULL;
    }

    return tp;
}

void thread_pool_destroy(struct thread_pool* tp)
{
    int i, num;

    if (!tp)
        return;

    num = tp->thread_num;
    tp->thread_num = 0; /* cannot add task any more */

    for (i = 0; i < num; ++i)
        do_add_task(tp, NULL, NULL);

    /* waiting for remaining tasks to finish */
    for (i = 0; i < num; ++i)
        pthread_join(tp->pidlist[i], NULL);

    thread_task_queue_destroy(&tp->queue);

    mm_free(tp);
}
