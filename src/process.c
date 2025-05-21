#include "process.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void initProcess(Process *process, int pid, int arrival_time, int burst_time, int priority) {
    process->pid = pid;
    process->arrival_time = arrival_time;
    process->burst_time = burst_time;
    process->priority = priority;
    process->remaining_time = burst_time;
    process->turnaround_time = 0;
    process->waiting_time = 0;
    process->response_time = -1;
}

Process *createProcess(int num_process, int max_arrival_time, int min_burst_time, int max_burst_time, int min_priority, int max_priority) {
    srand(time(NULL));
    Process *processes = (Process *)malloc(num_process * sizeof(Process));

    for(int i = 0; i < num_process; i++) {
        int arrival = rand() % max_arrival_time + 1;
        int burst = rand() % (max_burst_time - min_burst_time + 1) + min_burst_time;
        int priority = rand() % (max_priority - min_priority + 1) + min_priority;

        init_process(&processes[i], i+1, arrival, burst, priority);
    }

    return processes;
}

void print_processes(Process *processes, int num_processes) {
    printf("\n프로세스 목록:\n");
    printf("--------------------------------------------\n");
    printf("PID\t도착시간\t실행시간\t우선순위\n");
    printf("--------------------------------------------\n");

    for (int i = 0; i < num_processes; i++) {
        printf("%d\t%d\t\t%d\t\t%d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority);
    }
    printf("--------------------------------------------\n\n");
}

void free_processes(Process *processes) {
    free(processes);
}


