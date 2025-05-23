#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 프로세스 생성 함수
Process *create_processes(int count) {
    Process *processes = (Process *)malloc(sizeof(Process) * count);

    for (int i = 0; i < count; i++) {
        processes[i].pid = i;
        processes[i].arrival_time = rand() % 10;  // 0~9 사이 도착 시간
        processes[i].cpu_burst = rand() % 10 + 1; // 1~10 사이 CPU 버스트

        // I/O 처리
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
        processes[i].start = FALSE;
        processes[i].progress = 0;
        processes[i].comp_time = 0;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
    }

    return processes;
}

// 프로세스 상태 초기화 함수
void reset_processes(Process *processes, int count) {
    for (int i = 0; i < count; i++) {
        processes[i].remaining_time = processes[i].cpu_burst;
        processes[i].start = FALSE;
        processes[i].progress = 0;
        processes[i].comp_time = 0;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
    }
}
