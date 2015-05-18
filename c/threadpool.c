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

struct thread_node {
    pthread_t pid;
    struct list_node node;
    struct threadpool* tp;
};

static inline void thread_node_free(struct thread_node* node)
{
    free(node);
}

static void* thread_worker(void* arg)
{
    struct thread_node* myself = (struct thread_node*)arg;
    struct threadpool* tp = myself->tp;
    struct thread_task_queue* q = &(myself->tp->queue);

    while (1) {
        struct thread_task* t;

        t = thread_task_queue_dequeue(q);
        if (!t->func) {
            thread_task_free(t);

            pthread_mutex_lock(&tp->thread_list_lock);
            --tp->thread_num;
            __list_del(&myself->node);
            thread_node_free(myself);
            pthread_mutex_unlock(&tp->thread_list_lock);

            pthread_cond_signal(&tp->thread_list_cond);

            break;
        }

        t->func(t->arg);

        thread_task_free(t);
    }

    return NULL;
}

static inline struct thread_node* thread_node_alloc(struct threadpool* tp)
{
    struct thread_node* node = malloc(sizeof(struct thread_node));
    if (node) {
        node->tp = tp;
        list_init(&node->node);

        if (pthread_create(&node->pid, NULL, thread_worker, node) != 0) {
            free(node);
            return NULL;
        }

        pthread_detach(node->pid);
    }

    return node;
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
    struct thread_node* node = thread_node_alloc(tp);
    if (node) {
        pthread_mutex_lock(&tp->thread_list_lock);
        list_add_prev(&node->node, &tp->thread_list);
        ++tp->thread_num;
        pthread_mutex_unlock(&tp->thread_list_lock);
        return 0;
    }

    return -1;
}

void threadpool_add_thread(struct threadpool* tp, unsigned int num)
{
    for (; num > 0; --num)
        do_add_thread(tp);
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

struct threadpool* threadpool_init(unsigned int thread_num)
{
    unsigned int i;
    struct threadpool* tp;

    if (thread_num == 0)
        thread_num = sysconf(_SC_NPROCESSORS_CONF) - 1;

    tp = malloc(sizeof(struct threadpool));
    if (!tp)
        return NULL;

    tp->thread_num = 0;
    thread_task_queue_init(&tp->queue);
    pthread_mutex_init(&tp->thread_list_lock, NULL);
    pthread_cond_init(&tp->thread_list_cond, NULL);
    list_init(&tp->thread_list);

    for (i = 0; i < thread_num; ++i)
        do_add_thread(tp);

    if (tp->thread_num == 0) {
        free(tp);
        return NULL;
    }

    return tp;
}

void threadpool_destroy(struct threadpool* tp)
{
    unsigned int i, num;

    if (!tp)
        return;

    num = tp->thread_num;

    for (i = 0; i < num; ++i)
        do_del_thread(tp);

    /* waiting for remaining tasks to finish */
    pthread_mutex_lock(&tp->thread_list_lock);
    while (!list_empty(&tp->thread_list))
        pthread_cond_wait(&tp->thread_list_cond, &tp->thread_list_lock);
    pthread_mutex_unlock(&tp->thread_list_lock);

    pthread_cond_destroy(&tp->thread_list_cond);
    pthread_mutex_destroy(&tp->thread_list_lock);

    thread_task_queue_destroy(&tp->queue);

    free(tp);
}
