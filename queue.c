#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "queue.h"
#include <stdbool.h>

// Queue Struct
typedef struct queue {
    int in;
    int out;
    int size;
    int count;
    void **buffer;
    pthread_mutex_t lock;
    pthread_cond_t condition;
} queue_t;

// Allocation and Initialization
queue_t *queue_new(int size) {
    queue_t *q = malloc(sizeof(queue_t));
    assert(q != NULL);
    // Initialize queue
    q->in = 0;
    q->out = 0;
    q->size = size;
    q->count = 0;
    q->buffer = malloc(sizeof(void *) * size);
    assert(q->buffer != NULL);
    // pthread init
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->condition, NULL);
    return q;
}

// Free Memory
void queue_delete(queue_t **q) {
    if (*q != NULL) {
        free((*q)->buffer);
        pthread_mutex_destroy(&(*q)->lock);
        pthread_cond_destroy(&(*q)->condition);
        free(*q);
        *q = NULL;
    }
}

// Add Element to Queue
bool queue_push(queue_t *q, void *elem) {
    bool result = true;
    pthread_mutex_lock(&q->lock);
    while (q->count >= q->size) {
        pthread_cond_wait(&q->condition, &q->lock);
    }
    q->buffer[q->in] = elem;
    q->in = (q->in + 1) % q->size;
    q->count++;
    pthread_cond_broadcast(&q->condition);
    pthread_mutex_unlock(&q->lock);
    return result;
}

// Remove Element from Queue
bool queue_pop(queue_t *q, void **elem) {
    bool result = false;
    if (q != NULL && elem != NULL) {
        pthread_mutex_lock(&q->lock);
        while (q->count <= 0) {
            pthread_cond_wait(&q->condition, &q->lock);
        }
        *elem = q->buffer[q->out];
        q->out = (q->out + 1) % q->size;
        q->count--;
        pthread_cond_broadcast(&q->condition);
        pthread_mutex_unlock(&q->lock);
        result = true;
    }
    return result;
}
