#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"

/* ------------------------------------------------------------------------- */

struct thread_task {
    struct list_node node;
    void* arg;
    void (*func)(void*);
    void (*destructor)(void*);
};

static inline struct thread_task* thread_task_alloc(void* arg,
                                                    void (*func)(void*),
                                                    void (*destructor)(void*))
{
    struct thread_task* t;

    t = malloc(sizeof(struct thread_task));
    if (t) {
        t->arg = arg;
        t->func = func;
        t->destructor = destructor;
    }

    return t;
}

static inline void thread_task_free(struct thread_task* t)
{
    if (t->destructor)
        t->destructor(t->arg);

    free(t);
}

/* ------------------------------------------------------------------------- */

static inline void thread_task_queue_enqueue(struct thread_task_queue* q,
                                             struct thread_task* t)
{
    pthread_mutex_lock(&q->mutex);
    list_add_prev(&t->node, &q->tasklist);
    ++q->num;
    pthread_mutex_unlock(&q->mutex);
    pthread_cond_signal(&q->cond);
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
    --q->num;

    pthread_mutex_unlock(&q->mutex);

    return list_entry(t, struct thread_task, node);
}

static inline void thread_task_queue_init(struct thread_task_queue* q)
{
    q->num = 0;
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

    pthread_cond_destroy(&q->cond);
    pthread_mutex_destroy(&q->mutex);

    q->num = 0;
}

static inline unsigned int thread_task_queue_size(struct thread_task_queue* q)
{
    return q->num;
}

/* ------------------------------------------------------------------------- */

static void* thread_worker(void* arg)
{
    struct threadpool* tp = (struct threadpool*)arg;
    struct thread_task_queue* q = &(tp->queue);

    while (1) {
        struct thread_task* t;

        t = thread_task_queue_dequeue(q);
        if (!t->func) {
            thread_task_free(t);

            pthread_mutex_lock(&tp->thread_num_lock);
            --tp->thread_num;
            pthread_mutex_unlock(&tp->thread_num_lock);
            pthread_cond_signal(&tp->thread_num_cond);

            break;
        }

        t->func(t->arg);

        thread_task_free(t);
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */

unsigned int threadpool_task_num(struct threadpool* tp)
{
    return thread_task_queue_size(&tp->queue);
}

static inline int do_add_task(struct threadpool* tp, void* arg,
                              void (*func)(void*), void (*destructor)(void*))
{
    struct thread_task* task;

    task = thread_task_alloc(arg, func, destructor);
    if (!task)
        return -1;

    thread_task_queue_enqueue(&tp->queue, task);
    return 0;
}

int threadpool_add_task(struct threadpool* tp, void* arg,
                        void (*func)(void*), void (*destructor)(void*))
{
    if (!tp || tp->thread_num == 0 || !func)
        return -1;

    return do_add_task(tp, arg, func, destructor);
}

static inline int do_add_thread(struct threadpool* tp)
{
    pthread_t pid;
    if (pthread_create(&pid, NULL, thread_worker, tp) == 0) {
        pthread_detach(pid);
        pthread_mutex_lock(&tp->thread_num_lock);
        ++tp->thread_num;
        pthread_mutex_unlock(&tp->thread_num_lock);
        return 1;
    }

    return 0;
}

int threadpool_add_thread(struct threadpool* tp, unsigned int num)
{
    unsigned int i, counter = 0;

    for (i = 0; i < num; ++i)
        counter += do_add_thread(tp);

    if (counter == num) {
        return 0;
    }

    return -1;
}

static inline void do_del_thread(struct threadpool* tp)
{
    do_add_task(tp, NULL, NULL, NULL);
}

void threadpool_del_thread(struct threadpool* tp, unsigned int num)
{
    unsigned int i;

    if (num > tp->thread_num)
        num = tp->thread_num;

    for (i = 0; i < num; ++i)
        do_del_thread(tp);
}

int threadpool_init(struct threadpool* tp, unsigned int thread_num)
{
    unsigned int i;

    if (thread_num == 0)
        thread_num = sysconf(_SC_NPROCESSORS_CONF) - 1;

    tp->thread_num = 0;
    thread_task_queue_init(&tp->queue);
    pthread_mutex_init(&tp->thread_num_lock, NULL);
    pthread_cond_init(&tp->thread_num_cond, NULL);

    for (i = 0; i < thread_num; ++i)
        do_add_thread(tp);

    if (tp->thread_num == 0) {
        return -1;
    }

    return 0;
}

void threadpool_destroy(struct threadpool* tp)
{
    unsigned int i, num;

    num = tp->thread_num;
    for (i = 0; i < num; ++i)
        do_del_thread(tp);

    /* waiting for remaining tasks to finish */
    pthread_mutex_lock(&tp->thread_num_lock);
    while (tp->thread_num > 0)
        pthread_cond_wait(&tp->thread_num_cond, &tp->thread_num_lock);
    pthread_mutex_unlock(&tp->thread_num_lock);

    pthread_cond_destroy(&tp->thread_num_cond);
    pthread_mutex_destroy(&tp->thread_num_lock);

    thread_task_queue_destroy(&tp->queue);
}
