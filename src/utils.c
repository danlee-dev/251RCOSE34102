#include "utils.h"
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

void sort_io_operations(Process *p) {
    // 버블 정렬을 사용하여 io_start 기준으로 오름차순 정렬
    for (int i = 0; i < MAX_IO_OPERATIONS - 1; i++) {
        for (int j = 0; j < MAX_IO_OPERATIONS - i - 1; j++) {
            // 둘 다 유효한 I/O 작업인 경우에만 비교
            if (p->io_operations[j].io_start != -1 &&
                p->io_operations[j + 1].io_start != -1) {

                // io_start 기준으로 정렬 (작은 값이 앞에 오도록)
                if (p->io_operations[j].io_start >
                    p->io_operations[j + 1].io_start) {
                    // 두 I/O 작업 교환
                    IOOperation temp = p->io_operations[j];
                    p->io_operations[j] = p->io_operations[j + 1];
                    p->io_operations[j + 1] = temp;
                }
            }
        }
    }
}


// I/O 작업 추가 함수
void add_io_to_process(Process *p, int io_start, int io_burst) {
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start == -1) {
            p->io_operations[i].io_start = io_start;
            p->io_operations[i].io_burst = io_burst;
            break;
        }
    }
}

// I/O 작업 개수 반환
int get_io_count(Process *p) {
    int count = 0;
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start != -1) {
            count++;
        }
    }
    return count;
}


// 시간이 이미 사용되었는지 확인하는 함수
int is_time_used(int *used_times, int count, int time) {
    for (int i = 0; i < count; i++) {
        if (used_times[i] == time) {
            return 1;
        }
    }
    return 0;
}

// 특정 진행도에서 I/O가 시작되는지 확인
int has_io_at_progress(Process *p, int progress) {
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start == progress) {
            return 1;
        }
    }
    return 0;
}

// 특정 진행도에서의 I/O 버스트 시간 반환
int get_io_burst_at_progress(Process *p, int progress) {
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start == progress) {
            return p->io_operations[i].io_burst;
        }
    }
    return 0;
}
