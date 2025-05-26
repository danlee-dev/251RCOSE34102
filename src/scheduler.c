#include "scheduler.h"
#include "evaluation.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GanttEntrySize 1000

// 헬퍼 함수들
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

// FCFS 알고리즘 구현
Metrics *run_fcfs(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("FCFS Scheduling", 115);
    printf("\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q;
    init_queue(&ready_q);
    init_queue(&running_q);

    int *waiting_q = (int *)malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        waiting_q[i] = -1;
    }

    int time = 0;
    int completed = 0;
    int idle_time = 0;
    int pick = -1;

    while (completed < count) {
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                }
            }
        }

        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;

            if (processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;
                printf("waiting time, pid: %d, waiting time: %d, time: %d\n",
                       finished, processes[finished].waiting_time, time);
                completed++;
            }
        }
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) { // 프로세스 ID 유효성 검사
                    processes[pid].waiting_time_counter++;
                }
            }
        }
        time++;
    }
    metrics->total_time = time;
    metrics->idle_time = idle_time;

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "FCFS");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

// SJF(비선점) 알고리즘 구현
Metrics *run_sjf_np(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("Non-Preemptive SJF Scheduling", 115);
    printf("\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q, temp_q;
    init_queue(&ready_q);
    init_queue(&running_q);
    init_queue(&temp_q);

    int *waiting_q = (int *)malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        waiting_q[i] = -1;
    }

    int time = 0;
    int completed = 0;
    int idle_time = 0;
    int pick = -1;

    while (completed < count) {
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                if (is_empty(&ready_q)) {
                    enqueue(&ready_q, i);
                } else {
                    int inserted = 0;
                    while (!is_empty(&ready_q)) {
                        int current = peek(&ready_q);
                        if (!inserted &&
                            processes[i].remaining_time <
                                processes[current].remaining_time) {
                            enqueue(&temp_q, i);
                            inserted = 1;
                        }
                        enqueue(&temp_q, dequeue(&ready_q));
                    }

                    if (!inserted) {
                        enqueue(&temp_q, i);
                    }

                    while (!is_empty(&temp_q)) {
                        enqueue(&ready_q, dequeue(&temp_q));
                    }
                }
            }

            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    if (is_empty(&ready_q)) {
                        enqueue(&ready_q, i);
                    } else {
                        int inserted = 0;
                        while (!is_empty(&ready_q)) {
                            int current = peek(&ready_q);
                            if (!inserted &&
                                processes[i].cpu_burst - processes[i].progress <
                                    processes[current].cpu_burst -
                                        processes[current].progress) {
                                enqueue(&temp_q, i);
                                inserted = 1;
                            }
                            enqueue(&temp_q, dequeue(&ready_q));
                        }

                        if (!inserted) {
                            enqueue(&temp_q, i);
                        }

                        while (!is_empty(&temp_q)) {
                            enqueue(&ready_q, dequeue(&temp_q));
                        }
                    }
                    waiting_q[i] = -1;
                }
            }
        }

        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;
            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;
                completed++;
            }
        }
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) { // 프로세스 배열 크기로 검사
                    processes[pid].waiting_time_counter++;
                }
            }
        }
        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Non-Preemptive SJF");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

// SJF(선점) 알고리즘 구현
Metrics *run_sjf_p(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("Preemptive SJF Scheduling", 115);
    printf("\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q, temp_q;
    init_queue(&ready_q);
    init_queue(&running_q);
    init_queue(&temp_q);

    int *waiting_q = (int *)malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        waiting_q[i] = -1;
    }

    int time = 0;
    int completed = 0;
    int idle_time = 0;
    int pick = -1;
    int preempted = -1;

    while (completed < count) {
        for (int i = 0; i < count; i++) {

            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    if (!is_empty(&running_q)) {
                        int current = peek(&running_q);
                        int remaining_current = processes[current].cpu_burst -
                                                processes[current].progress;
                        int remaining_i =
                            processes[i].cpu_burst - processes[i].progress;

                        if (remaining_i < remaining_current) {
                            preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            if (is_empty(&ready_q)) {
                                enqueue(&ready_q, i);
                            } else {
                                int inserted = 0;
                                while (!is_empty(&ready_q)) {
                                    int current = peek(&ready_q);
                                    if (!inserted &&
                                        (processes[i].cpu_burst -
                                         processes[i].progress) <
                                            (processes[current].cpu_burst -
                                             processes[current].progress)) {
                                        enqueue(&temp_q, i);
                                        inserted = 1;
                                    }
                                    enqueue(&temp_q, dequeue(&ready_q));
                                }

                                if (!inserted) {
                                    enqueue(&temp_q, i);
                                }

                                while (!is_empty(&temp_q)) {
                                    enqueue(&ready_q, dequeue(&temp_q));
                                }
                            }
                        }
                    } else {
                        enqueue(&running_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }

            if (processes[i].arrival_time == time) {
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);
                    if (processes[i].remaining_time <
                        processes[current].remaining_time) {
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                    } else {
                        if (is_empty(&ready_q)) {
                            enqueue(&ready_q, i);
                        } else {
                            int inserted = 0;
                            while (!is_empty(&ready_q)) {
                                int current = peek(&ready_q);
                                if (!inserted &&
                                    processes[i].remaining_time <
                                        processes[current].remaining_time) {
                                    enqueue(&temp_q, i);
                                    inserted = 1;
                                }
                                enqueue(&temp_q, dequeue(&ready_q));
                            }

                            if (!inserted) {
                                enqueue(&temp_q, i);
                            }

                            while (!is_empty(&temp_q)) {
                                enqueue(&ready_q, dequeue(&temp_q));
                            }
                        }
                    }
                } else if (is_empty(&ready_q)) {
                    enqueue(&running_q, i);
                } else {
                    int inserted = 0;
                    while (!is_empty(&ready_q)) {
                        int current = peek(&ready_q);
                        if (!inserted &&
                            processes[i].remaining_time <
                                processes[current].remaining_time) {
                            enqueue(&temp_q, i);
                            inserted = 1;
                        }
                        enqueue(&temp_q, dequeue(&ready_q));
                    }

                    if (!inserted) {
                        enqueue(&temp_q, i);
                    }

                    while (!is_empty(&temp_q)) {
                        enqueue(&ready_q, dequeue(&temp_q));
                    }
                }
            }
        }

        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;
            processes[pick].remaining_time =
                processes[pick].cpu_burst - processes[pick].progress;

            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;
                completed++;
            }
        }
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) { // 프로세스 배열 크기로 검사
                    processes[pid].waiting_time_counter++;
                }
            }
        }
        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Preemptive SJF");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_priority_np(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("Non-Preemptive Priority Scheduling", 115);
    printf("\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q, temp_q;
    init_queue(&ready_q);
    init_queue(&running_q);
    init_queue(&temp_q);

    int *waiting_q = (int *)malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        waiting_q[i] = -1;
    }

    int time = 0;
    int completed = 0;
    int idle_time = 0;
    int pick = -1;

    while (completed < count) {
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                if (is_empty(&ready_q)) {
                    enqueue(&ready_q, i);
                } else {
                    int inserted = 0;
                    while (!is_empty(&ready_q)) {
                        int current = peek(&ready_q);
                        if (!inserted && processes[i].priority <
                                             processes[current].priority) {
                            enqueue(&temp_q, i);
                            inserted = 1;
                        }
                        enqueue(&temp_q, dequeue(&ready_q));
                    }

                    if (!inserted) {
                        enqueue(&temp_q, i);
                    }

                    while (!is_empty(&temp_q)) {
                        enqueue(&ready_q, dequeue(&temp_q));
                    }
                }
            }
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    if (is_empty(&ready_q)) {
                        enqueue(&ready_q, i);
                    } else {
                        int inserted = 0;
                        while (!is_empty(&ready_q)) {
                            int current = peek(&ready_q);
                            if (!inserted && processes[i].priority <
                                                 processes[current].priority) {
                                enqueue(&temp_q, i);
                                inserted = 1;
                            }
                            enqueue(&temp_q, dequeue(&ready_q));
                        }

                        if (!inserted) {
                            enqueue(&temp_q, i);
                        }

                        while (!is_empty(&temp_q)) {
                            enqueue(&ready_q, dequeue(&temp_q));
                        }
                    }
                    waiting_q[i] = -1;
                }
            }
        }

        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;
            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;
                completed++;
            }
        }
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) { // 프로세스 배열 크기로 검사
                    processes[pid].waiting_time_counter++;
                }
            }
        }
        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Non-Preemptive Priority");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

// 선점형 우선순위 스케줄링 알고리즘 구현
Metrics *run_priority_p(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("Preemptive Priority Scheduling", 115);
    printf("\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q, temp_q;
    init_queue(&ready_q);
    init_queue(&running_q);
    init_queue(&temp_q);

    int *waiting_q = (int *)malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        waiting_q[i] = -1;
    }

    int time = 0;
    int completed = 0;
    int idle_time = 0;
    int pick = -1;
    int preempted = -1;

    while (completed < count) {
        for (int i = 0; i < count; i++) {

            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    if (!is_empty(&running_q)) {
                        int current = peek(&running_q);

                        if (processes[i].priority <
                            processes[current].priority) {
                            preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            if (is_empty(&ready_q)) {
                                enqueue(&ready_q, i);
                            } else {
                                int inserted = 0;
                                while (!is_empty(&ready_q)) {
                                    int current = peek(&ready_q);
                                    if (!inserted &&
                                        processes[i].priority <
                                            processes[current].priority) {
                                        enqueue(&temp_q, i);
                                        inserted = 1;
                                    }
                                    enqueue(&temp_q, dequeue(&ready_q));
                                }

                                if (!inserted) {
                                    enqueue(&temp_q, i);
                                }

                                while (!is_empty(&temp_q)) {
                                    enqueue(&ready_q, dequeue(&temp_q));
                                }
                            }
                        }
                    } else {
                        enqueue(&running_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }

            if (processes[i].arrival_time == time) {
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);

                    if (processes[i].priority < processes[current].priority) {
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                    } else {
                        if (is_empty(&ready_q)) {
                            enqueue(&ready_q, i);
                        } else {
                            int inserted = 0;
                            while (!is_empty(&ready_q)) {
                                int current = peek(&ready_q);
                                if (!inserted &&
                                    processes[i].priority <
                                        processes[current].priority) {
                                    enqueue(&temp_q, i);
                                    inserted = 1;
                                }
                                enqueue(&temp_q, dequeue(&ready_q));
                            }

                            if (!inserted) {
                                enqueue(&temp_q, i);
                            }

                            while (!is_empty(&temp_q)) {
                                enqueue(&ready_q, dequeue(&temp_q));
                            }
                        }
                    }
                } else if (is_empty(&ready_q)) {
                    enqueue(&running_q, i);
                } else {
                    int inserted = 0;
                    while (!is_empty(&ready_q)) {
                        int current = peek(&ready_q);
                        if (!inserted && processes[i].priority <
                                             processes[current].priority) {
                            enqueue(&temp_q, i);
                            inserted = 1;
                        }
                        enqueue(&temp_q, dequeue(&ready_q));
                    }

                    if (!inserted) {
                        enqueue(&temp_q, i);
                    }

                    while (!is_empty(&temp_q)) {
                        enqueue(&ready_q, dequeue(&temp_q));
                    }
                }
            }
        }

        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;

            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;
                completed++;
            }
        }
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) { // 프로세스 배열 크기로 검사
                    processes[pid].waiting_time_counter++;
                }
            }
        }
        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Preemptive Priority");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_rr(Process *processes, int count, Config *config) {
    printf("\n");
    print_thin_emphasized_header("Round Robin Scheduling", 115);
    printf("\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q;
    init_queue(&ready_q);
    init_queue(&running_q);

    int *waiting_q = (int *)malloc(sizeof(int) * count);
    int *time_quantum = (int *)malloc(sizeof(int) * count);

    for (int i = 0; i < count; i++) {
        waiting_q[i] = -1;
        time_quantum[i] = 0;
    }

    int time = 0;
    int completed = 0;
    int idle_time = 0;
    int pick = -1;
    int quantum = config->time_quantum;

    printf("** Time Quantum: %d **\n", quantum);

    while (completed < count) {
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                    time_quantum[i] = 0;
                }
            }
        }

        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;
            time_quantum[pick]++;

            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
                time_quantum[waiting] = 0;
            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;
                completed++;
                time_quantum[finished] = 0;
            } else if (time_quantum[pick] == quantum) {
                if (!is_empty(&ready_q)) {
                    int rotated = dequeue(&running_q);
                    enqueue(&ready_q, rotated);
                    time_quantum[rotated] = 0;
                } else {
                    time_quantum[pick] = 0;
                }
            }
        }
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front; i <= ready_q.rear; i++) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }
        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Round Robin");

    free(gantt.entries);
    free(waiting_q);
    free(time_quantum);

    return metrics;
}

Metrics *run_priority_with_aging(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("Priority Scheduling with Aging", 115);
    printf("\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q, temp_q;
    init_queue(&ready_q);
    init_queue(&running_q);
    init_queue(&temp_q);

    Process *p_copy = (Process *)malloc(sizeof(Process) * count);
    // 기존 프로세스 정보 유지를 위한 복제(아 포인터였음 **주의 -> processes가
    // 변경되면 p도 변경됨)

    memcpy(p_copy, processes, sizeof(Process) * count);

    int *waiting_q = (int *)malloc(sizeof(int) * count);
    int *age = (int *)malloc(sizeof(int) * count);

    for (int i = 0; i < count; i++) {
        waiting_q[i] = -1;
        age[i] = 0;
    }

    const int AGING_THRESHOLD = 3;

    int time = 0;
    int completed = 0;
    int idle_time = 0;
    int pick = -1;
    int preempted = -1;

    printf("\n** Aging Information (Threshold: %d) **\n", AGING_THRESHOLD);
    printf("+------+-------------+---------------+-----------------+\n");
    printf("| Time | Process ID  | Current Age   | Priority Change |\n");
    printf("+------+-------------+---------------+-----------------+\n");

    while (completed < count) {
        int aging_candidates[100];
        int aging_count = 0;

        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    age[pid]++;
                    if (age[pid] >= AGING_THRESHOLD &&
                        processes[pid].priority > 1) {
                        aging_candidates[aging_count++] = pid;
                    }
                }
            }
        }

        for (int i = 0; i < aging_count; i++) {
            int pid = aging_candidates[i];
            int old_priority = processes[pid].priority;
            processes[pid].priority--;

            printf("| %-4d | P%-10d | %-13d | %7d → %-6d|\n", time, pid,
                   age[pid], old_priority, processes[pid].priority);

            age[pid] = 0;
        }

        if (aging_count > 0) {
            Queue new_ready_q;
            init_queue(&new_ready_q);

            while (!is_empty(&ready_q)) {
                enqueue(&temp_q, dequeue(&ready_q));
            }

            while (!is_empty(&temp_q)) {
                int current_pid = dequeue(&temp_q);

                if (is_empty(&new_ready_q)) {
                    enqueue(&new_ready_q, current_pid);
                } else {
                    Queue temp_q2;
                    init_queue(&temp_q2);
                    int inserted = 0;

                    while (!is_empty(&new_ready_q)) {
                        int next_pid = peek(&new_ready_q);
                        if (!inserted && processes[current_pid].priority <
                                             processes[next_pid].priority) {
                            enqueue(&temp_q2, current_pid);
                            inserted = 1;
                        }
                        enqueue(&temp_q2, dequeue(&new_ready_q));
                    }

                    if (!inserted) {
                        enqueue(&temp_q2, current_pid);
                    }

                    while (!is_empty(&temp_q2)) {
                        enqueue(&new_ready_q, dequeue(&temp_q2));
                    }
                }
            }

            ready_q = new_ready_q;
        }

        if (!is_empty(&running_q) && !is_empty(&ready_q)) {
            int running_pid = peek(&running_q);
            int top_ready_pid = peek(&ready_q);

            if (processes[top_ready_pid].priority <
                processes[running_pid].priority) {
                int preempted = dequeue(&running_q);
                enqueue(&ready_q, preempted);

                int promoted = dequeue(&ready_q);
                enqueue(&running_q, promoted);
                age[promoted] = 0;

                // printf("| %4d | P%-10d |     (Aging 선점 발생)   |\n", time,
                //        promoted);
            }
        }

        for (int i = 0; i < count; i++) {
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    if (!is_empty(&running_q)) {
                        int current = peek(&running_q);

                        if (processes[i].priority <
                            processes[current].priority) {
                            preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                            age[i] = 0;
                        } else {
                            if (is_empty(&ready_q)) {
                                enqueue(&ready_q, i);
                            } else {
                                int inserted = 0;
                                while (!is_empty(&ready_q)) {
                                    int current = peek(&ready_q);
                                    if (!inserted &&
                                        processes[i].priority <
                                            processes[current].priority) {
                                        enqueue(&temp_q, i);
                                        inserted = 1;
                                    }
                                    enqueue(&temp_q, dequeue(&ready_q));
                                }

                                if (!inserted) {
                                    enqueue(&temp_q, i);
                                }

                                while (!is_empty(&temp_q)) {
                                    enqueue(&ready_q, dequeue(&temp_q));
                                }
                            }
                        }
                    } else {
                        enqueue(&running_q, i);
                        age[i] = 0;
                    }
                    waiting_q[i] = -1;
                }
            }

            if (processes[i].arrival_time == time) {
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);

                    if (processes[i].priority < processes[current].priority) {
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                        age[i] = 0;
                    } else {
                        if (is_empty(&ready_q)) {
                            enqueue(&ready_q, i);
                        } else {
                            int inserted = 0;
                            while (!is_empty(&ready_q)) {
                                int current = peek(&ready_q);
                                if (!inserted &&
                                    processes[i].priority <
                                        processes[current].priority) {
                                    enqueue(&temp_q, i);
                                    inserted = 1;
                                }
                                enqueue(&temp_q, dequeue(&ready_q));
                            }

                            if (!inserted) {
                                enqueue(&temp_q, i);
                            }

                            while (!is_empty(&temp_q)) {
                                enqueue(&ready_q, dequeue(&temp_q));
                            }
                        }
                    }
                } else if (is_empty(&ready_q)) {
                    enqueue(&running_q, i);
                    age[i] = 0;
                } else {
                    int inserted = 0;
                    while (!is_empty(&ready_q)) {
                        int current = peek(&ready_q);
                        if (!inserted && processes[i].priority <
                                             processes[current].priority) {
                            enqueue(&temp_q, i);
                            inserted = 1;
                        }
                        enqueue(&temp_q, dequeue(&ready_q));
                    }

                    if (!inserted) {
                        enqueue(&temp_q, i);
                    }

                    while (!is_empty(&temp_q)) {
                        enqueue(&ready_q, dequeue(&temp_q));
                    }
                }
            }
        }

        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
            age[pick] = 0;
        }

        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;

            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;
                completed++;
            }
        }

        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        time++;
    }

    printf("+------+-------------+---------------+-----------------+\n\n");

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    for (int i = 0; i < count; i++) {
        processes[i].priority = p_copy[i].priority;
    }

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Priority with Aging");

    free(gantt.entries);
    free(waiting_q);
    free(age);

    return metrics;
}

Metrics *run_rms(Process *processes, int count, Config *config) {
    printf("\n");
    print_thin_emphasized_header("Rate Monotonic Scheduling", 115);
    printf("\n");

    Metrics *metrics = malloc(sizeof(Metrics));
    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q, temp_q;
    init_queue(&ready_q);
    init_queue(&running_q);
    init_queue(&temp_q);

    Process *original_processes = malloc(sizeof(Process) * count);
    memcpy(original_processes, processes, sizeof(Process) * count);

    Process *arrival_tracker = malloc(sizeof(Process) * count);
    memcpy(arrival_tracker, processes, sizeof(Process) * count);

    DeadlineMissInfo *deadline_miss_info =
        malloc(sizeof(DeadlineMissInfo) * count * 10);
    config->deadline_miss_info_count = 0;

    int *io_remaining = malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        io_remaining[i] = -1;
    }

    int time = 0;
    int idle_time = 0;
    int has_missed = 0;

    printf("\n** Process Period Information (Lower Period = Higher Priority) "
           "**\n");
    printf("+------+-------------+------------+------------+------------+------"
           "------+-----+\n");
    printf("| PID  |   Arrival   |   Period   | Deadline 1 | Deadline 2 | "
           "Deadline 3 | ... |\n");
    printf("+------+-------------+------------+------------+------------+------"
           "------+-----+\n");
    for (int i = 0; i < count; i++) {
        printf("| P%-3d | %-11d | %-10d | %-10d | %-10d | %-10d | ... |\n", i,
               original_processes[i].arrival_time, original_processes[i].period,
               original_processes[i].deadline,
               original_processes[i].deadline + original_processes[i].period,
               original_processes[i].deadline +
                   2 * original_processes[i].period);
    }
    printf("+------+-------------+------------+------------+------------+------"
           "------+-----+\n\n");

    printf("\n** Deadline Misses Log **\n");
    printf("+------+-------------+------------------+-----------------+\n");
    printf("| Time | Process ID  | Absolute Deadline| Completion Time |\n");
    printf("+------+-------------+------------------+-----------------+\n");

    while (time < 90) {
        int *arrived_processes = malloc(sizeof(int) * count);
        int arrival_count = 0;

        // 현재 시간에 도착한 프로세스 수집
        for (int i = 0; i < count; i++) {
            if (arrival_tracker[i].arrival_time == time) {
                arrived_processes[arrival_count++] = i;
                arrival_tracker[i].arrival_time += arrival_tracker[i].period;
                arrival_tracker[i].deadline += arrival_tracker[i].period;
            }
        }

        // 주기 기준으로 정렬 (짧은 주기가 높은 우선순위)
        for (int i = 0; i < arrival_count - 1; i++) {
            for (int j = 0; j < arrival_count - i - 1; j++) {
                if (processes[arrived_processes[j]].period >
                    processes[arrived_processes[j + 1]].period) {
                    int temp = arrived_processes[j];
                    arrived_processes[j] = arrived_processes[j + 1];
                    arrived_processes[j + 1] = temp;
                }
            }
        }

        // 새로운 프로세스 도착 처리
        for (int idx = 0; idx < arrival_count; idx++) {
            int process_id = arrived_processes[idx];

            if (!is_empty(&running_q)) {
                int current_running = peek(&running_q);
                if (processes[process_id].period <
                    processes[current_running].period) {
                    int preempted = dequeue(&running_q);
                    enqueue(&ready_q, preempted);
                    enqueue(&running_q, process_id);
                } else {
                    // 주기 기준으로 ready_q에 삽입
                    if (is_empty(&ready_q)) {
                        enqueue(&ready_q, process_id);
                    } else {
                        int inserted = 0;
                        while (!is_empty(&ready_q)) {
                            int current = peek(&ready_q);
                            if (!inserted && processes[process_id].period <
                                                 processes[current].period) {
                                enqueue(&temp_q,
                                        process_id); // 주가 짧은 프로세스를
                                                     // 먼저 삽입
                                inserted = 1;
                            }
                            enqueue(&temp_q, dequeue(&ready_q));
                        }
                        if (!inserted) {
                            enqueue(&temp_q, process_id);
                        }
                        while (!is_empty(&temp_q)) {
                            enqueue(&ready_q, dequeue(&temp_q));
                        }
                    }
                }
            } else if (is_empty(&ready_q)) {
                enqueue(&running_q, process_id);
            } else {
                // 주기 기준으로 ready_q에 삽입, 삽입 정렬
                int inserted = 0;
                while (!is_empty(&ready_q)) {
                    int current = peek(&ready_q);
                    if (!inserted && processes[process_id].period <
                                         processes[current].period) {
                        enqueue(&temp_q, process_id);
                        inserted = 1;
                    }
                    enqueue(&temp_q, dequeue(&ready_q));
                }
                if (!inserted) {
                    enqueue(&temp_q, process_id);
                }
                while (!is_empty(&temp_q)) {
                    enqueue(&ready_q, dequeue(&temp_q));
                }
            }
        }

        free(arrived_processes);

        // I/O 완료 처리
        for (int i = 0; i < count; i++) {
            if (io_remaining[i] > 0) {
                io_remaining[i]--;
                if (io_remaining[i] == 0) {
                    if (!is_empty(&running_q)) {
                        int current_running = peek(&running_q);
                        if (processes[i].period <
                            processes[current_running].period) {
                            int preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            // 주기 기준으로 ready_q에 삽입
                            if (is_empty(&ready_q)) {
                                enqueue(&ready_q, i);
                            } else {
                                int inserted = 0;
                                while (!is_empty(&ready_q)) {
                                    int current = peek(&ready_q);
                                    if (!inserted &&
                                        processes[i].period <
                                            processes[current].period) {
                                        enqueue(&temp_q, i);
                                        inserted = 1;
                                    }
                                    enqueue(&temp_q, dequeue(&ready_q));
                                }
                                if (!inserted) {
                                    enqueue(&temp_q, i);
                                }
                                while (!is_empty(&temp_q)) {
                                    enqueue(&ready_q, dequeue(&temp_q));
                                }
                            }
                        }
                    } else {
                        enqueue(&running_q, i);
                    }
                    io_remaining[i] = -1;
                }
            }
        }

        // Ready queue에서 프로세스 가져오기
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            int next_process = dequeue(&ready_q);
            enqueue(&running_q, next_process);
        }

        // 프로세스 실행
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            int current_process = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, current_process, "RUN");

            processes[current_process].progress++;
            processes[current_process].remaining_time =
                processes[current_process].cpu_burst -
                processes[current_process].progress;

            // I/O 요청 처리
            if (processes[current_process].io_burst > 0 &&
                processes[current_process].progress ==
                    processes[current_process].io_start) {
                int io_process = dequeue(&running_q);
                io_remaining[io_process] = processes[io_process].io_burst;
            }
            // 프로세스 완료 처리
            else if (processes[current_process].progress ==
                     processes[current_process].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time +=
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time +=
                    processes[finished].waiting_time_counter;

                processes[finished].progress = 0;

                // 데드라인 미스 체크
                if (processes[finished].comp_time >
                    processes[finished].deadline) {
                    deadline_miss_info[config->deadline_miss_info_count].pid =
                        finished;
                    deadline_miss_info[config->deadline_miss_info_count]
                        .absolute_deadline = processes[finished].deadline;
                    deadline_miss_info[config->deadline_miss_info_count]
                        .completion_time = processes[finished].comp_time;
                    processes[finished].missed_deadline = 1;
                    config->deadline_miss_info_count++;
                    has_missed = 1;

                    printf("| %4d | P%-10d | %-16d | %-15d |\n", time + 1,
                           finished, processes[finished].deadline,
                           processes[finished].comp_time);
                }

                // 다음 주기로 업데이트
                processes[finished].arrival_time += processes[finished].period;
                processes[finished].deadline += processes[finished].period;
                processes[finished].io_start += processes[finished].period;
            }
        }

        // Ready queue 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        time++;
    }

    if (!has_missed) {
        printf("|                There is no deadline miss                |\n");
    }
    printf("+------+-------------+------------------+-----------------+\n\n");

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    printf("\n** Total Deadline Misses: %d **\n",
           config->deadline_miss_info_count);

    // 원본 프로세스 정보 복구
    for (int i = 0; i < count; i++) {
        processes[i].arrival_time = original_processes[i].arrival_time;
        processes[i].deadline = original_processes[i].deadline;
        processes[i].io_start = original_processes[i].io_start;
    }

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Rate Monotonic Scheduling");

    free(gantt.entries);
    free(io_remaining);
    free(original_processes);
    free(arrival_tracker);
    free(deadline_miss_info);

    return metrics;
}

Metrics *run_edf(Process *processes, int count, Config *config) {
    printf("\n");
    print_thin_emphasized_header("Earliest Deadline First Scheduling", 115);
    printf("\n");

    Metrics *metrics = malloc(sizeof(Metrics));
    reset_processes(processes, count);

    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * GanttEntrySize);
    gantt.count = 0;
    gantt.capacity = GanttEntrySize;

    Queue ready_q, running_q;
    init_queue(&ready_q);
    init_queue(&running_q);

    Process *original_processes = malloc(sizeof(Process) * count);
    memcpy(original_processes, processes, sizeof(Process) * count);

    int *next_arrival = malloc(sizeof(int) * count);
    int *next_deadline = malloc(sizeof(int) * count);

    for (int i = 0; i < count; i++) {
        next_arrival[i] = processes[i].arrival_time;
        next_deadline[i] = processes[i].deadline;
    }

    DeadlineMissInfo *deadline_miss_info =
        malloc(sizeof(DeadlineMissInfo) * count * 10);
    config->deadline_miss_info_count = 0;

    int *io_remaining = malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        io_remaining[i] = -1;
    }

    int time = 0;
    int idle_time = 0;

    printf("\n** Process Deadline Information **\n");
    printf("+------+-------------+------------+------------+------------+------"
           "------+-----+\n");
    printf("| PID  |   Arrival   |   Period   | Deadline 1 | Deadline 2 | "
           "Deadline 3 | ... |\n");
    printf("+------+-------------+------------+------------+------------+------"
           "------+-----+\n");
    for (int i = 0; i < count; i++) {
        printf("| P%-3d | %-11d | %-10d | %-10d | %-10d | %-10d | ... |\n", i,
               original_processes[i].arrival_time, original_processes[i].period,
               original_processes[i].deadline,
               original_processes[i].deadline + original_processes[i].period,
               original_processes[i].deadline +
                   2 * original_processes[i].period);
    }
    printf("+------+-------------+------------+------------+------------+------"
           "------+-----+\n\n");

    printf("** Deadline Misses Log **\n");
    printf("+------+-------------+------------------+-----------------+\n");
    printf("| Time | Process ID  | Absolute Deadline| Completion Time |\n");
    printf("+------+-------------+------------------+-----------------+\n");

    int has_missed = 0;

    while (time < 90) {
        // I/O 완료 처리
        for (int i = 0; i < count; i++) {
            if (io_remaining[i] > 0) {
                io_remaining[i]--;
                if (io_remaining[i] == 0) {

                    // I/O 완료 후 스케줄링 결정
                    if (!is_empty(&running_q)) {
                        int current_running = peek(&running_q);
                        if (processes[i].deadline <
                            processes[current_running].deadline) {
                            int preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            insert_by_deadline(&ready_q, i, processes);
                        }
                    } else {
                        enqueue(&running_q, i);
                    }
                    io_remaining[i] = -1;
                }
            }
        }

        // 새로운 프로세스 도착 처리
        for (int i = 0; i < count; i++) {
            if (next_arrival[i] == time && io_remaining[i] == -1) {
                processes[i].progress = 0;
                processes[i].remaining_time = processes[i].cpu_burst;
                processes[i].deadline = next_deadline[i];
                processes[i].arrival_time = time;
                processes[i].waiting_time_counter = 0;

                next_arrival[i] += processes[i].period;
                next_deadline[i] += processes[i].period;

                // 도착 시 스케줄링 결정
                if (!is_empty(&running_q)) {
                    int current_running = peek(&running_q);
                    if (processes[i].deadline <
                        processes[current_running].deadline) {
                        int preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                    } else {
                        insert_by_deadline(&ready_q, i, processes);
                    }
                } else if (is_empty(&ready_q)) {
                    enqueue(&running_q, i);
                } else {
                    insert_by_deadline(&ready_q, i, processes);
                }
            }
        }

        // Ready queue에서 프로세스 가져오기
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            int next_process =
                get_earliest_deadline_process(&ready_q, processes);
            remove_from_queue(&ready_q, next_process);
            enqueue(&running_q, next_process);
        }

        // 프로세스 실행
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            int current_running = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, current_running, "RUN");

            processes[current_running].progress++;
            processes[current_running].remaining_time--;

            // I/O 요청 처리
            if (processes[current_running].io_burst > 0 &&
                processes[current_running].progress ==
                    processes[current_running].io_start) {

                int io_process = current_running;
                io_remaining[io_process] = processes[io_process].io_burst;
                dequeue(&running_q);
            }
            // 프로세스 완료 처리
            else if (processes[current_running].progress ==
                     processes[current_running].cpu_burst) {
                int finished = current_running;
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time +=
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time +=
                    processes[finished].waiting_time_counter;

                processes[finished].progress = 0;

                // 데드라인 미스 체크
                if (processes[finished].comp_time >
                    processes[finished].deadline) {
                    deadline_miss_info[config->deadline_miss_info_count].pid =
                        finished;
                    deadline_miss_info[config->deadline_miss_info_count]
                        .absolute_deadline = processes[finished].deadline;
                    deadline_miss_info[config->deadline_miss_info_count]
                        .completion_time = processes[finished].comp_time;

                    processes[finished].missed_deadline = 1;
                    config->deadline_miss_info_count++;
                    has_missed = 1;

                    printf("| %4d | P%-10d | %-16d | %-15d |\n", time + 1,
                           finished, processes[finished].deadline,
                           processes[finished].comp_time);
                }

                // 다음 주기로 업데이트
                processes[finished].arrival_time += processes[finished].period;
                processes[finished].deadline += processes[finished].period;
                processes[finished].io_start += processes[finished].period;

                dequeue(&running_q);
            }
        }

        // Ready queue의 프로세스들만 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                processes[pid].waiting_time_counter++;
            }
        }

        time++;
    }

    if (!has_missed) {
        printf("|                There is no deadline miss                |\n");
    }
    printf("+------+-------------+------------------+-----------------+\n\n");

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    printf("\n** Total Deadline Misses: %d **\n",
           config->deadline_miss_info_count);

    // 복구
    for (int i = 0; i < count; i++) {
        processes[i].arrival_time = original_processes[i].arrival_time;
        processes[i].deadline = original_processes[i].deadline;
        processes[i].io_start = original_processes[i].io_start;
        processes[i].period = original_processes[i].period;
        processes[i].cpu_burst = original_processes[i].cpu_burst;
        processes[i].io_burst = original_processes[i].io_burst;
        processes[i].priority = original_processes[i].priority;
    }

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Earliest Deadline First");

    free(gantt.entries);
    free(original_processes);
    free(next_arrival);
    free(next_deadline);
    free(deadline_miss_info);
    free(io_remaining);

    return metrics;
}
