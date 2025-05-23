#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


Process *create_processes(int count, char mode) {
    Process *processes = (Process *)malloc(sizeof(Process) * count);

    if (mode == 'y') {
        for (int i = 0; i < count; i++) {
            processes[i].pid = i;
            processes[i].arrival_time = rand() % 10;  // 0~9 사이 도착 시간
            processes[i].cpu_burst = rand() % 10 + 1; // 1~10 사이 CPU 버스트

            if (processes[i].cpu_burst > 2) {
                processes[i].io_start = rand() % (processes[i].cpu_burst - 1) +
                                        1; // CPU 버스트 중간에 I/O 요청
                processes[i].io_burst = rand() % 5 + 1; // 1~5 사이 I/O 버스트
            } else {
                processes[i].io_start = -1; // I/O 요청 없음
                processes[i].io_burst = 0;
            }

            processes[i].priority = rand() % 10 + 1; // 1~10 사이 우선순위
            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
        }
    } else {
        for (int i = 0; i < count; i++) {
            printf("Enter process %d information:\n", i);

            printf("  PID: ");
            scanf("%d", &processes[i].pid);

            printf("  Arrival Time: ");
            scanf("%d", &processes[i].arrival_time);

            printf("  CPU Burst: ");
            scanf("%d", &processes[i].cpu_burst);

            printf("  IO Start: ");
            scanf("%d", &processes[i].io_start);

            printf("  IO Burst: ");
            scanf("%d", &processes[i].io_burst);

            printf("  Priority: ");
            scanf("%d", &processes[i].priority);

            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
        }
    }

    return processes;
}

void reset_processes(Process *processes, int count) {
    for (int i = 0; i < count; i++) {
        processes[i].remaining_time = processes[i].cpu_burst;
        processes[i].progress = 0;
        processes[i].comp_time = 0;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time_counter = 0;
    }
}
