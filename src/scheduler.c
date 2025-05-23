#include "scheduler.h"
#include "evaluation.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define GanttEntrySize 1000

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
                completed++;
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
                        if (!inserted && processes[i].cpu_burst <
                                             processes[current].cpu_burst) {
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
                    int remaining_current = processes[current].cpu_burst -
                                            processes[current].progress;
                    int remaining_i = processes[i].cpu_burst;

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
                                    processes[i].cpu_burst <
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
                } else if (is_empty(&ready_q)) {
                    enqueue(&running_q, i);
                } else {
                    int inserted = 0;
                    while (!is_empty(&ready_q)) {
                        int current = peek(&ready_q);
                        if (!inserted && processes[i].cpu_burst <
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
