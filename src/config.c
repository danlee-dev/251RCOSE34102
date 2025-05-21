#include <queue.h>

void Config(Queue readyQueue, Queue waitQueue, Queue RunningQueue) {
    InitQueue(&readyQueue);
    InitQueue(&waitQueue);
    InitQueue(&RunningQueue);
    printf("Queue initialized successfully.\n");
}
