#ifndef __queue_h__
#define __queue_h__

typedef int Data;

typedef struct _node {
    Data data;
    struct _node* next;
} Node;

typedef struct _queue {
    Node* front;
    Node* rear;
} Queue;

void InitQueue(Queue* q);
void Enqueue(Queue* q, Data data);
Data Dequeue(Queue* q);
int IsEmpty(Queue* q);
int peek(Queue* q);

#endif

