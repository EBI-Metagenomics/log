#include "log/log.h"
#include "log/spinlock.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define QUEUE_SIZE 16

static void default_print(char const *msg, void *arg) { printf("%s\n", msg); }
static void default_flush(void *arg) { fflush(stdout); }

struct log_queue
{
    char const *_Atomic msg[QUEUE_SIZE];
    atomic_uint count;
    atomic_uint head;
    atomic_uint tail;
    spinlock_t lock;
    atomic_bool flush_now;
    int level;
    log_print_t *print;
    log_flush_t *flush;
    void *callback_arg;
} log_queue = {
    .count = 0,
    .head = 0,
    .tail = 0,
    .lock = SPINLOCK_INIT,
    .flush_now = ATOMIC_BOOL_LOCK_FREE,
    .level = LOG_INFO,
    .print = default_print,
    .flush = default_flush,
    .callback_arg = NULL,
};

static char const *__pop(void)
{
    struct log_queue *q = &log_queue;
    char const *msg = atomic_exchange(&q->msg[q->tail], NULL);
    if (!msg)
        return NULL;

    if (++q->tail >= QUEUE_SIZE)
        q->tail = 0;

    size_t r = atomic_fetch_sub(&q->count, 1);
    assert(r > 0);

    return msg;
}

static bool __put(char const *msg)
{
    struct log_queue *q = &log_queue;
    if (atomic_load(&q->flush_now))
    {
        q->print(msg, q->callback_arg);
        q->flush(q->callback_arg);
        return true;
    }
    size_t count = atomic_fetch_add(&q->count, 1);
    if (count >= QUEUE_SIZE)
    {
        /* back off, queue is full */
        atomic_fetch_sub(&q->count, 1);
        return false;
    }

    /* increment the head, which gives us 'exclusive' access to that element */
    size_t head = atomic_fetch_add(&q->head, 1);
    assert(q->msg[head % QUEUE_SIZE] == 0);
    char const *rv = atomic_exchange(&q->msg[head % QUEUE_SIZE], msg);
    assert(rv == NULL);
    return true;
}

static void flush_weak(void)
{
    struct log_queue *q = &log_queue;
    spinlock_lock(&q->lock);

    char const *msg = NULL;
    while ((msg = __pop()))
        q->print(msg, q->callback_arg);
    q->flush(q->callback_arg);

    spinlock_unlock(&q->lock);
}

void log_flush(void)
{
    atomic_store(&log_queue.flush_now, true);
    flush_weak();
}

void __log_put(int level, char const *msg)
{
    if (level < log_queue.level)
        return;

    while (!__put(msg))
        flush_weak();
}

void log_setup(int level, log_print_t *print, log_flush_t *flush, void *arg)
{
    log_queue.level = level;
    log_queue.print = print;
    log_queue.flush = flush;
    log_queue.callback_arg = arg;
}
