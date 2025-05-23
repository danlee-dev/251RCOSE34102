#include "queue.h"

// 큐 초기화
void init_queue(Queue *q) {
    q->front = 0;
    q->rear = -1;
}

// 큐가 비어있는지 확인
int is_empty(Queue *q) {
    return (q->rear < q->front);
}

// 큐가 가득 찼는지 확인
int is_full(Queue *q) {
    return (q->rear >= MAX_QUEUE_SIZE - 1);
}

// 큐에 데이터 추가
void enqueue(Queue *q, int value) {
    if (!is_full(q)) {
        q->data[++q->rear] = value;
    }
}

// 큐에서 데이터 제거 및 반환
int dequeue(Queue *q) {
    if (!is_empty(q)) {
        return q->data[q->front++];
    }
    return -1; // 큐가 비어있을 경우
}

// 큐의 첫 번째 데이터 확인
int peek(Queue *q) {
    if (!is_empty(q)) {
        return q->data[q->front];
    }
    return -1; // 큐가 비어있을 경우
}
