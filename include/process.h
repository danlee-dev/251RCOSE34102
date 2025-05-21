#ifndef __process_h__
#define __process_h__

typedef struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int io_burst_time;
    int preemption_time;
} Process;

#endif
