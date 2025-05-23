#include "queue.h"

void init_queue(Queue *q) {
    q->front = 0;
    q->rear = -1;
}

int is_empty(Queue *q) {
    return (q->rear < q->front);
}

int is_full(Queue *q) {
    return (q->rear >= MAX_QUEUE_SIZE - 1);
}

void enqueue(Queue *q, int value) {
    if (!is_full(q)) {
        q->data[++q->rear] = value;
    }
}

int dequeue(Queue *q) {
    if (!is_empty(q)) {
        return q->data[q->front++];
    }
    return -1;
}

int peek(Queue *q) {
    if (!is_empty(q)) {
        return q->data[q->front];
    }
    return -1;
}
