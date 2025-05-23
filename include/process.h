#ifndef PROCESS_H
#define PROCESS_H

#define TRUE 1
#define FALSE 0

typedef struct {
    int pid;             // 프로세스 ID
    int arrival_time;    // 도착 시간
    int cpu_burst;       // CPU 버스트 시간
    int io_start;        // I/O 요청 시점(CPU 버스트 중)
    int io_burst;        // I/O 버스트 시간
    int priority;        // 우선순위 (낮을수록 높은 우선순위)
    int remaining_time;  // 남은 실행 시간(선점형 알고리즘용)
    int progress;        // 프로세스 진행 상태
    int comp_time;       // 완료 시간
    int waiting_time;    // 대기 시간
    int turnaround_time; // 반환 시간
    int waiting_time_counter; // 대기 시간 카운터
} Process;

Process *create_processes(int count, char mode);
void reset_processes(Process *processes, int count);

#endif
