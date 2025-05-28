#include "sort_utils.h"
#include <stdlib.h>

// 두 프로세스를 비교하는 함수 (작은 값이 우선순위가 높음)
int compare_processes(Process *processes, int pid1, int pid2, SortCriteria criteria) {
    switch (criteria) {
        case SORT_BY_PRIORITY:
            return processes[pid1].priority - processes[pid2].priority;

        case SORT_BY_REMAINING_TIME:
            return (processes[pid1].cpu_burst - processes[pid1].progress) -
                   (processes[pid2].cpu_burst - processes[pid2].progress);

        case SORT_BY_DEADLINE:
            return processes[pid1].deadline - processes[pid2].deadline;

        case SORT_BY_PERIOD:
            return processes[pid1].period - processes[pid2].period;

        default:
            return 0;
    }
}

// 큐를 정렬하는 함수
void sort_queue(Queue *queue, Process *processes, SortCriteria criteria) {
    if (is_empty(queue) || queue->count <= 1) {
        return;  // 빈 큐이거나 원소가 1개 이하면 정렬할 필요 없음
    }

    // 큐의 모든 원소를 임시 배열에 복사
    int *temp_array = (int *)malloc(sizeof(int) * queue->count);
    int temp_count = 0;

    // 큐에서 모든 원소를 꺼내서 배열에 저장
    while (!is_empty(queue)) {
        temp_array[temp_count++] = dequeue(queue);
    }

    // 버블 정렬로 배열 정렬 (간단한 구현)
    for (int i = 0; i < temp_count - 1; i++) {
        for (int j = 0; j < temp_count - i - 1; j++) {
            if (compare_processes(processes, temp_array[j], temp_array[j + 1], criteria) > 0) {
                // 교환
                int temp = temp_array[j];
                temp_array[j] = temp_array[j + 1];
                temp_array[j + 1] = temp;
            }
        }
    }

    // 정렬된 배열을 다시 큐에 삽입
    for (int i = 0; i < temp_count; i++) {
        enqueue(queue, temp_array[i]);
    }

    free(temp_array);
}

// 큐에 새로운 프로세스를 정렬된 위치에 삽입하는 함수
void insert_sorted(Queue *queue, Process *processes, int pid, SortCriteria criteria) {
    if (is_empty(queue)) {
        enqueue(queue, pid);
        return;
    }

    Queue temp_queue;
    init_queue(&temp_queue);

    int inserted = 0;

    // 큐를 순회하면서 적절한 위치를 찾아 삽입
    while (!is_empty(queue)) {
        int current = peek(queue);

        if (!inserted && compare_processes(processes, pid, current, criteria) <= 0) {
            enqueue(&temp_queue, pid);
            inserted = 1;
        }

        enqueue(&temp_queue, dequeue(queue));
    }

    // 끝까지 삽입되지 않았다면 맨 뒤에 삽입
    if (!inserted) {
        enqueue(&temp_queue, pid);
    }

    // 임시 큐의 내용을 원본 큐로 복사
    while (!is_empty(&temp_queue)) {
        enqueue(queue, dequeue(&temp_queue));
    }
}

// 편의 함수들 (기존 코드와의 호환성을 위해)
void sort_ready_queue_by_priority(Queue *ready_q, Process *processes) {
    sort_queue(ready_q, processes, SORT_BY_PRIORITY);
}

void sort_ready_queue_by_remaining_time(Queue *ready_q, Process *processes) {
    sort_queue(ready_q, processes, SORT_BY_REMAINING_TIME);
}

void sort_ready_queue_by_deadline(Queue *ready_q, Process *processes) {
    sort_queue(ready_q, processes, SORT_BY_DEADLINE);
}


void insert_by_deadline(Queue *queue, int pid, Process *processes) {
    if (is_empty(queue)) {
        enqueue(queue, pid);
        return;
    }

    Queue temp_queue;
    init_queue(&temp_queue);
    int inserted = 0;

    while (!is_empty(queue)) {
        int current_pid = dequeue(queue);

        if (!inserted &&
            processes[pid].deadline < processes[current_pid].deadline) {
            enqueue(&temp_queue, pid);
            inserted = 1;
        }
        enqueue(&temp_queue, current_pid);
    }

    if (!inserted) {
        enqueue(&temp_queue, pid);
    }

    while (!is_empty(&temp_queue)) {
        enqueue(queue, dequeue(&temp_queue));
    }
}

int get_earliest_deadline_process(Queue *queue, Process *processes) {
    if (is_empty(queue))
        return -1;

    int min_deadline = processes[queue->data[queue->front]].deadline;
    int selected_pid = queue->data[queue->front];

    for (int i = queue->front, cnt = 0; cnt < queue->count;
         cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        int pid = queue->data[i];
        if (processes[pid].deadline < min_deadline) {
            min_deadline = processes[pid].deadline;
            selected_pid = pid;
        }
    }

    return selected_pid;
}

// Period 기준으로 ready queue에 프로세스를 정렬된 위치에 삽입
void insert_by_period(Queue *queue, int pid, Process *processes) {
    if (is_empty(queue)) {
        enqueue(queue, pid);
        return;
    }

    Queue temp_queue;
    init_queue(&temp_queue);
    int inserted = 0;

    while (!is_empty(queue)) {
        int current_pid = dequeue(queue);

        if (!inserted &&
            processes[pid].period < processes[current_pid].period) {
            enqueue(&temp_queue, pid);
            inserted = 1;
        }
        enqueue(&temp_queue, current_pid);
    }

    if (!inserted) {
        enqueue(&temp_queue, pid);
    }

    while (!is_empty(&temp_queue)) {
        enqueue(queue, dequeue(&temp_queue));
    }
}

// 가장 짧은 period를 가진 프로세스를 찾아서 반환
int get_shortest_period_process(Queue *queue, Process *processes) {
    if (is_empty(queue))
        return -1;

    int min_period = processes[queue->data[queue->front]].period;
    int selected_pid = queue->data[queue->front];

    for (int i = queue->front, cnt = 0; cnt < queue->count;
         cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        int pid = queue->data[i];
        if (processes[pid].period < min_period) {
            min_period = processes[pid].period;
            selected_pid = pid;
        }
    }

    return selected_pid;
}


void remove_from_queue(Queue *queue, int pid) {
    Queue temp_queue;
    init_queue(&temp_queue);

    while (!is_empty(queue)) {
        int current_pid = dequeue(queue);
        if (current_pid != pid) {
            enqueue(&temp_queue, current_pid);
        }
    }

    while (!is_empty(&temp_queue)) {
        enqueue(queue, dequeue(&temp_queue));
    }
}
