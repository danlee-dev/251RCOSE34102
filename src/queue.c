#include "queue.h"

void init_queue(Queue *q) {
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

int is_empty(Queue *q) {
    return (q->count == 0);
}

int is_full(Queue *q) {
    return (q->count == MAX_QUEUE_SIZE);
}

void enqueue(Queue *q, int value) {
    if (!is_full(q)) {
        q->data[q->rear] = value;
        q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
        q->count++;
    }
}

int dequeue(Queue *q) {
    if (!is_empty(q)) {
        int value = q->data[q->front];
        q->front = (q->front + 1) % MAX_QUEUE_SIZE;
        q->count--;
        return value;
    }
    return -1;
}

int peek(Queue *q) {
    if (!is_empty(q)) {
        return q->data[q->front];
    }
    return -1;
}
