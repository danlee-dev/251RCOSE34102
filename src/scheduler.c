#include "scheduler.h"
#include "evaluation.h"
#include "queue.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GanttEntrySize 1000


Metrics *run_fcfs(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("FCFS Scheduling with Multi-I/O", 150);
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
            // 도착 프로세스 처리
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            // I/O 완료 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                }
            }
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        // 프로세스 실행 및 상태 변경
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            processes[pick].progress++;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&processes[pick],
                                   processes[pick].progress)) {
                int waiting = dequeue(&running_q);
                int io_burst = get_io_burst_at_progress(
                    &processes[waiting], processes[waiting].progress);
                waiting_q[waiting] = io_burst + 1;

            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                // 프로세스 완료
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

        // 대기 큐 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        // 디버깅용 ready queue 상태 출력 (간소화)
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue: ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d ", pid);
        //     }
        //     printf("\n");
        // }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("\nScheduling completed at time %d\n", time);
    // printf("Total idle time: %d\n", idle_time);

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "FCFS Multi-I/O");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_sjf_np(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("Non-Preemptive SJF Scheduling with Multi-I/O",
                                 150);
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
        // 도착 프로세스 처리 및 I/O 완료 처리
        for (int i = 0; i < count; i++) {
            // 도착 프로세스 처리
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            // I/O 완료 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                }
            }
        }

        // Ready queue를 남은 실행시간 기준으로 정렬 (SJF)
        if (!is_empty(&ready_q)) {
            sort_queue(&ready_q, processes, SORT_BY_REMAINING_TIME);
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        // 프로세스 실행 및 상태 변경
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            processes[pick].progress++;
            processes[pick].remaining_time =
                processes[pick].cpu_burst - processes[pick].progress;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&processes[pick],
                                   processes[pick].progress)) {
                int waiting = dequeue(&running_q);
                int io_burst = get_io_burst_at_progress(
                    &processes[waiting], processes[waiting].progress);
                waiting_q[waiting] = io_burst + 1;

            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                // 프로세스 완료
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

        // 대기 큐 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        // 디버깅용 ready queue 상태 출력 (간소화)
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue: ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(rem:%d) ", pid,
        //                processes[pid].cpu_burst - processes[pid].progress);
        //     }
        //     printf("\n");
        // }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("\nNon-Preemptive SJF scheduling completed at time %d\n", time);
    // printf("Total idle time: %d\n", idle_time);

    // 평균 통계 계산 및 출력
    // double avg_waiting = 0, avg_turnaround = 0;
    // for (int i = 0; i < count; i++) {
    //     avg_waiting += processes[i].waiting_time;
    //     avg_turnaround += processes[i].turnaround_time;
    // }
    // avg_waiting /= count;
    // avg_turnaround /= count;

    // printf("Average Waiting Time: %.2f\n", avg_waiting);
    // printf("Average Turnaround Time: %.2f\n", avg_turnaround);

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Non-Preemptive SJF Multi-I/O");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_sjf_p(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header("Preemptive SJF Scheduling with Multi-I/O",
                                 150);
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
        // 도착 프로세스 처리 및 I/O 완료 처리
        for (int i = 0; i < count; i++) {

            // 도착 프로세스 처리 및 선점 여부 처리
            if (processes[i].arrival_time == time) {
                // 새로 도착한 프로세스로 선점 검사
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);
                    if (processes[i].cpu_burst - processes[i].progress <
                        processes[current].cpu_burst -
                            processes[current].progress) {
                        // 선점 발생
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                    } else {
                        // ready queue에 추가
                        enqueue(&ready_q, i);
                    }
                } else {
                    // ready queue에 추가
                    enqueue(&ready_q, i);
                }
            }

            // I/O 완료 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 선점 검사
                    if (!is_empty(&running_q)) {
                        int current = peek(&running_q);
                        int remaining_current = processes[current].cpu_burst -
                                                processes[current].progress;
                        int remaining_i =
                            processes[i].cpu_burst - processes[i].progress;

                        if (remaining_i < remaining_current) {
                            // 선점 발생
                            preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            // ready queue에 추가
                            enqueue(&ready_q, i);
                        }
                    } else {
                        // running queue가 비어있으면 바로 실행
                        enqueue(&ready_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }
        }
        if (!is_empty(&ready_q)) {
            sort_queue(&ready_q, processes, SORT_BY_REMAINING_TIME);
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        // 프로세스 실행 및 상태 변경
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;
            processes[pick].remaining_time =
                processes[pick].cpu_burst - processes[pick].progress;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&processes[pick],
                                   processes[pick].progress)) {
                int waiting = dequeue(&running_q);
                int io_burst = get_io_burst_at_progress(
                    &processes[waiting], processes[waiting].progress);
                waiting_q[waiting] = io_burst + 1;

            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                // 프로세스 완료
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

        // 대기 큐 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        // 디버깅용 ready queue 상태 출력 (간소화)
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue: ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(rem:%d) ", pid,
        //                processes[pid].cpu_burst - processes[pid].progress);
        //     }
        //     printf("\n");
        // }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("\nPreemptive SJF scheduling completed at time %d\n", time);
    // printf("Total idle time: %d\n", idle_time);

    // 평균 통계 계산 및 출력
    // double avg_waiting = 0, avg_turnaround = 0;
    // for (int i = 0; i < count; i++) {
    //     avg_waiting += processes[i].waiting_time;
    //     avg_turnaround += processes[i].turnaround_time;
    // }
    // avg_waiting /= count;
    // avg_turnaround /= count;

    // printf("Average Waiting Time: %.2f\n", avg_waiting);
    // printf("Average Turnaround Time: %.2f\n", avg_turnaround);

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Preemptive SJF Multi-I/O");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_priority_np(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header(
        "Non-Preemptive Priority Scheduling with Multi-I/O", 150);
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
        // 도착 프로세스 처리 및 I/O 완료 처리
        for (int i = 0; i < count; i++) {
            // 도착 프로세스 처리
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            // I/O 완료 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                }
            }
        }

        // Ready queue를 우선순위 기준으로 정렬
        if (!is_empty(&ready_q)) {
            sort_queue(&ready_q, processes, SORT_BY_PRIORITY);
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        // 프로세스 실행 및 상태 변경
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            processes[pick].progress++;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&processes[pick],
                                   processes[pick].progress)) {
                int waiting = dequeue(&running_q);
                int io_burst = get_io_burst_at_progress(
                    &processes[waiting], processes[waiting].progress);
                waiting_q[waiting] = io_burst + 1;

            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                // 프로세스 완료
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

        // 대기 큐 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        // 디버깅용 ready queue 상태 출력 (간소화)
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue (Priority order): ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(pri:%d) ", pid, processes[pid].priority);
        //     }
        //     printf("\n");
        // }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("\nNon-Preemptive Priority scheduling completed at time %d\n",
    // time); printf("Total idle time: %d\n", idle_time);

    // 평균 통계 계산 및 출력
    // double avg_waiting = 0, avg_turnaround = 0;
    // for (int i = 0; i < count; i++) {
    //     avg_waiting += processes[i].waiting_time;
    //     avg_turnaround += processes[i].turnaround_time;
    // }
    // avg_waiting /= count;
    // avg_turnaround /= count;

    // printf("Average Waiting Time: %.2f\n", avg_waiting);
    // printf("Average Turnaround Time: %.2f\n", avg_turnaround);

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Non-Preemptive Priority Multi-I/O");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_priority_p(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header(
        "Preemptive Priority Scheduling with Multi-I/O", 150);
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
        // 도착 프로세스 처리 및 I/O 완료 처리
        for (int i = 0; i < count; i++) {
            // 도착 프로세스 처리 및 선점 여부 처리
            if (processes[i].arrival_time == time) {
                // 새로 도착한 프로세스로 선점 검사
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);
                    if (processes[i].priority < processes[current].priority) {
                        // 선점 발생
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                    } else {
                        // ready queue에 추가
                        enqueue(&ready_q, i);
                    }
                } else {
                    // ready queue에 추가
                    enqueue(&ready_q, i);
                }
            }

            // I/O 완료 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 선점 검사
                    if (!is_empty(&running_q)) {
                        int current = peek(&running_q);
                        if (processes[i].priority <
                            processes[current].priority) {
                            // 선점 발생
                            preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            // ready queue에 추가
                            enqueue(&ready_q, i);
                        }
                    } else {
                        // running queue가 비어있으면 바로 실행
                        enqueue(&ready_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }
        }

        // Ready queue를 우선순위 기준으로 정렬
        if (!is_empty(&ready_q)) {
            sort_queue(&ready_q, processes, SORT_BY_PRIORITY);
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        // 프로세스 실행 및 상태 변경
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            processes[pick].progress++;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&processes[pick],
                                   processes[pick].progress)) {
                int waiting = dequeue(&running_q);
                int io_burst = get_io_burst_at_progress(
                    &processes[waiting], processes[waiting].progress);
                waiting_q[waiting] = io_burst + 1;

            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                // 프로세스 완료
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

        // 대기 큐 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        // 디버깅용 ready queue 상태 출력 (간소화)
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue (Priority order): ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(pri:%d) ", pid, processes[pid].priority);
        //     }
        //     printf("\n");
        // }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("\nPreemptive Priority scheduling completed at time %d\n", time);
    // printf("Total idle time: %d\n", idle_time);

    // 평균 통계 계산 및 출력
    // double avg_waiting = 0, avg_turnaround = 0;
    // for (int i = 0; i < count; i++) {
    //     avg_waiting += processes[i].waiting_time;
    //     avg_turnaround += processes[i].turnaround_time;
    // }
    // avg_waiting /= count;
    // avg_turnaround /= count;

    // printf("Average Waiting Time: %.2f\n", avg_waiting);
    // printf("Average Turnaround Time: %.2f\n", avg_turnaround);

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Preemptive Priority Multi-I/O");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_rr(Process *processes, int count, Config *config) {
    printf("\n");
    print_thin_emphasized_header("Round Robin Scheduling with Multi-I/O", 150);
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

    printf("** Time Quantum: %d **\n\n", quantum);

    while (completed < count) {
        // 타임 퀀텀 만료 처리 (새로운 프로세스 도착 전에 먼저 처리)
        if (!is_empty(&running_q) &&
            time_quantum[peek(&running_q)] == quantum) {
            int rotated = dequeue(&running_q);
            enqueue(&ready_q, rotated);
            time_quantum[rotated] = 0;
        }

        // 도착 프로세스 처리 및 I/O 완료 처리
        for (int i = 0; i < count; i++) {
            // 도착 프로세스 처리
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            // I/O 완료 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                    time_quantum[i] = 0; // 타임 퀀텀 리셋
                }
            }
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
        }

        // 프로세스 실행 및 상태 변경
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            processes[pick].progress++;
            time_quantum[pick]++;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&processes[pick],
                                   processes[pick].progress)) {
                int waiting = dequeue(&running_q);
                int io_burst = get_io_burst_at_progress(
                    &processes[waiting], processes[waiting].progress);
                waiting_q[waiting] = io_burst + 1;
                time_quantum[waiting] = 0; // 타임 퀀텀 리셋

            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                // 프로세스 완료
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].waiting_time_counter;

                completed++;
                time_quantum[finished] = 0;
            }
        }

        // 대기 큐 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        // 디버깅용 ready queue 상태 출력 (간소화)
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue: ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(prog:%d/%d) ", pid, processes[pid].progress,
        //                processes[pid].cpu_burst);
        //     }
        //     printf("\n");
        // }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("\nRound Robin scheduling completed at time %d\n", time);
    // printf("Total idle time: %d\n", idle_time);

    // // Round Robin 성능 분석
    // printf("\nRound Robin Performance Analysis:\n");
    // printf("Time Quantum: %d\n", quantum);

    // // 평균 통계 계산 및 출력
    // double avg_waiting = 0, avg_turnaround = 0;

    // for (int i = 0; i < count; i++) {
    //     avg_waiting += processes[i].waiting_time;
    //     avg_turnaround += processes[i].turnaround_time;
    // }
    // avg_waiting /= count;
    // avg_turnaround /= count;

    // printf("Average Waiting Time: %.2f\n", avg_waiting);
    // printf("Average Turnaround Time: %.2f\n", avg_turnaround);

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Round Robin Multi-I/O");

    free(gantt.entries);
    free(waiting_q);
    free(time_quantum);

    return metrics;
}

Metrics *run_priority_with_aging(Process *processes, int count) {
    printf("\n");
    print_thin_emphasized_header(
        "Priority Scheduling with Aging(Preemptive) and Multi-I/O", 150);
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
    // 기존 프로세스 정보 유지를 위한 복제
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
        // AGING 대상자 찾기 및 나이 증가
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

        // 나이 증가 후 우선순위 변경
        for (int i = 0; i < aging_count; i++) {
            int pid = aging_candidates[i];
            int old_priority = processes[pid].priority;
            processes[pid].priority--;

            printf("| %-4d | P%-10d | %-13d | %7d → %-6d|\n", time, pid,
                   age[pid], old_priority, processes[pid].priority);

            age[pid] = 0;
        }

        // 우선순위 변경 후 ready queue 재정렬 (aging이 발생한 경우에만)
        if (aging_count > 0 && !is_empty(&ready_q)) {
            sort_queue(&ready_q, processes, SORT_BY_PRIORITY);
        }

        // 선점 처리 (Aging 후)
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

                printf(
                    "| %-4d | P%-10d |     (Aging preemption occurred)   |\n",
                    time, promoted);
            }
        }

        // 도착 프로세스 처리 (I/O 완료 처리보다 먼저)
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                // 새로 도착한 프로세스로 선점 검사
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);
                    if (processes[i].priority < processes[current].priority) {
                        // 선점 발생
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                        age[i] = 0;
                    } else {
                        // ready queue에 추가
                        enqueue(&ready_q, i);
                    }
                } else {
                    // running queue가 비어있어도 ready queue에 추가
                    enqueue(&ready_q, i);
                }
            }
        }

        // I/O 완료 처리
        for (int i = 0; i < count; i++) {
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 선점 검사
                    if (!is_empty(&running_q)) {
                        int current = peek(&running_q);
                        if (processes[i].priority <
                            processes[current].priority) {
                            // 선점 발생
                            preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                            age[i] = 0;
                        } else {
                            // ready queue에 추가
                            enqueue(&ready_q, i);
                        }
                    } else {
                        // running queue가 비어있어도 ready queue에 추가
                        enqueue(&ready_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }
        }

        // Ready queue를 우선순위 기준으로 정렬 (CPU 할당 직전에 일괄 정렬)
        if (!is_empty(&ready_q)) {
            sort_queue(&ready_q, processes, SORT_BY_PRIORITY);
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
            age[pick] = 0;
        }

        // 프로세스 실행 및 상태 변경
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            pick = peek(&running_q);
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            processes[pick].progress++;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&processes[pick],
                                   processes[pick].progress)) {
                int waiting = dequeue(&running_q);
                int io_burst = get_io_burst_at_progress(
                    &processes[waiting], processes[waiting].progress);
                waiting_q[waiting] = io_burst + 1;

            } else if (processes[pick].progress == processes[pick].cpu_burst) {
                // 프로세스 완료
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

        // 대기 큐 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                if (pid >= 0 && pid < count) {
                    processes[pid].waiting_time_counter++;
                }
            }
        }

        // 디버깅용 ready queue 상태 출력 (간소화)
        // if (!is_empty(&ready_q)) { // 5시간마다 출력
        //     printf("Time %d Ready Queue: ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(pri:%d)", pid, processes[pid].priority);
        //     }
        //     printf("\n");
        // }

        time++;
    }

    printf("+------+-------------+---------------+-----------------+\n\n");

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // Aging 효과 분석
    // printf("** Aging Analysis **\n");
    // printf("Aging Threshold: %d time units\n", AGING_THRESHOLD);
    // for (int i = 0; i < count; i++) {
    //     printf("P%d: Initial Priority %d → Final Priority %d (Change: %d)\n",
    //     i,
    //            p_copy[i].priority, processes[i].priority,
    //            p_copy[i].priority - processes[i].priority);
    // }

    // 원래 우선순위로 복원
    for (int i = 0; i < count; i++) {
        processes[i].priority = p_copy[i].priority;
    }

    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Priority with Aging Multi-I/O");

    free(gantt.entries);
    free(waiting_q);
    free(age);
    free(p_copy);

    return metrics;
}

Metrics *run_rms(Process *processes, int count, Config *config, int max_time) {
    printf("\n");
    print_thin_emphasized_header("Rate Monotonic Scheduling with Multi-I/O",
                                 150);
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

    // 원본 프로세스 정보 저장
    Process *original_processes = malloc(sizeof(Process) * count);
    memcpy(original_processes, processes, sizeof(Process) * count);

    // 동적으로 확장 가능한 프로세스 배열
    int max_processes = count * 20; // 충분한 공간 할당
    Process *all_processes = malloc(sizeof(Process) * max_processes);
    int total_process_count = 0;

    // 초기 프로세스들은 실제 arrival_time에 도착하므로 여기서 추가하지 않음
    // next_arrival을 초기 arrival_time으로 설정
    int *next_arrival = malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        next_arrival[i] = original_processes[i].arrival_time;
    }

    DeadlineMissInfo *deadline_miss_info =
        malloc(sizeof(DeadlineMissInfo) * count * 10);
    config->deadline_miss_info_count = 0;

    int *waiting_q = malloc(sizeof(int) * max_processes);
    for (int i = 0; i < max_processes; i++) {
        waiting_q[i] = -1;
    }

    int time = 0;
    int idle_time = 0;

    print_utilization_analysis(original_processes, count, "RMS");

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

    printf("** Deadline Misses Log **\n");
    printf("+------+-------------+------------------+-----------------+\n");
    printf("| Time | Process ID  | Absolute Deadline| Completion Time |\n");
    printf("+------+-------------+------------------+-----------------+\n");

    int has_missed = 0;

    while (time < max_time) {
        // 새로운 프로세스 인스턴스 생성 (초기 도착 + 주기적 도착)
        for (int i = 0; i < count; i++) {
            if (next_arrival[i] == time) {
                // 새로운 프로세스 인스턴스 생성
                Process new_process;
                memcpy(&new_process, &original_processes[i], sizeof(Process));

                // 새로운 인스턴스의 속성 설정 (PID는 원본과 동일하게 유지)
                new_process.pid = i; // 원본 프로세스와 동일한 PID 유지
                new_process.arrival_time = time;

                // 첫 번째 도착인지 주기적 도착인지 확인하여 deadline 설정
                if (time == original_processes[i].arrival_time) {
                    // 첫 번째 도착 - 원본 deadline 사용
                    new_process.deadline = original_processes[i].deadline;
                } else {
                    // 주기적 도착 - deadline 계산
                    int period_count =
                        (time - original_processes[i].arrival_time) /
                        original_processes[i].period;
                    new_process.deadline =
                        original_processes[i].deadline +
                        period_count * original_processes[i].period;
                }

                new_process.progress = 0;
                new_process.waiting_time_counter = 0;
                new_process.comp_time = 0;
                new_process.waiting_time = 0;
                new_process.turnaround_time = 0;
                new_process.missed_deadline = 0;

                // all_processes 배열에 추가
                all_processes[total_process_count] = new_process;

                // 다음 도착 시간 업데이트
                next_arrival[i] += original_processes[i].period;

                // 스케줄링 결정 (period 기준)
                if (!is_empty(&running_q)) {
                    int current_running = peek(&running_q);
                    if (all_processes[total_process_count].period <
                        all_processes[current_running].period) {
                        int preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, total_process_count);
                    } else {
                        // 단순 enqueue (나중에 일괄 정렬)
                        enqueue(&ready_q, total_process_count);
                    }
                } else {
                    // 단순 enqueue (나중에 일괄 정렬)
                    enqueue(&ready_q, total_process_count);
                }

                total_process_count++;
            }
        }

        // I/O 완료 처리
        for (int i = 0; i < total_process_count; i++) {
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 스케줄링 결정
                    if (!is_empty(&running_q)) {
                        int current_running = peek(&running_q);
                        if (all_processes[i].period <
                            all_processes[current_running].period) {
                            int preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            enqueue(&ready_q, i);
                        }
                    } else {
                        enqueue(&ready_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }
        }

        // Ready queue를 period 기준으로 정렬 (CPU 할당 직전에 일괄 정렬)
        if (!is_empty(&ready_q)) {
            sort_queue(&ready_q, all_processes, SORT_BY_PERIOD);
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            int next_process =
                dequeue(&ready_q); // 이미 정렬되어 있으므로 첫 번째가 최단 주기
            enqueue(&running_q, next_process);
        }

        // 프로세스 실행
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            int current_running = peek(&running_q);
            // Gantt 차트에는 실제 PID를 사용
            add_gantt_entry(&gantt, time, time + 1,
                            all_processes[current_running].pid, "RUN");

            all_processes[current_running].progress++;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&all_processes[current_running],
                                   all_processes[current_running].progress)) {
                int io_process = current_running;
                int io_burst = get_io_burst_at_progress(
                    &all_processes[io_process],
                    all_processes[io_process].progress);
                waiting_q[io_process] = io_burst + 1;
                dequeue(&running_q);
            }
            // 프로세스 완료 처리
            else if (all_processes[current_running].progress ==
                     all_processes[current_running].cpu_burst) {
                int finished = current_running;
                all_processes[finished].comp_time = time + 1;
                all_processes[finished].turnaround_time =
                    all_processes[finished].comp_time -
                    all_processes[finished].arrival_time;
                all_processes[finished].waiting_time =
                    all_processes[finished].waiting_time_counter;

                // 데드라인 미스 체크
                if (all_processes[finished].comp_time >
                    all_processes[finished].deadline) {
                    deadline_miss_info[config->deadline_miss_info_count].pid =
                        all_processes[finished].pid; // 실제 PID 사용
                    deadline_miss_info[config->deadline_miss_info_count]
                        .absolute_deadline = all_processes[finished].deadline;
                    deadline_miss_info[config->deadline_miss_info_count]
                        .completion_time = all_processes[finished].comp_time;

                    all_processes[finished].missed_deadline = 1;
                    config->deadline_miss_info_count++;
                    has_missed = 1;

                    printf("| %4d | P%-10d | %-16d | %-15d |\n", time + 1,
                           all_processes[finished].pid,
                           all_processes[finished].deadline,
                           all_processes[finished].comp_time);
                }

                dequeue(&running_q);
            }
        }

        // Ready queue의 프로세스들만 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                all_processes[pid].waiting_time_counter++;
            }
        }

        // 디버깅용 ready queue 상태 출력
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue (Period order): ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(pr:%d) ", all_processes[pid].pid,
        //                all_processes[pid].period);
        //     }
        // }

        time++;
    }
    if (!has_missed) {
        printf("|                There is no deadline miss                |\n");
    }
    printf("+------+-------------+------------------+-----------------+\n\n");

    metrics->total_time = time;
    metrics->idle_time = idle_time;
    metrics->for_edf_rms_processes = all_processes;
    metrics->for_edf_rms_counter = total_process_count;

    printf("** Total Deadline Misses: %d **\n",
           config->deadline_miss_info_count);

    // 모든 프로세스 인스턴스를 포함하여 결과 출력
    display_scheduling_results(all_processes, total_process_count, &gantt, time,
                               idle_time,
                               "Rate Monotonic Scheduling Multi-I/O");

    free(gantt.entries);
    free(original_processes);
    free(all_processes);
    free(next_arrival);
    free(deadline_miss_info);
    free(waiting_q);

    return metrics;
}

Metrics *run_edf(Process *processes, int count, Config *config, int max_time) {
    printf("\n");
    print_thin_emphasized_header(
        "Earliest Deadline First Scheduling with Multi-I/O", 150);
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

    // 원본 프로세스 정보 저장
    Process *original_processes = malloc(sizeof(Process) * count);
    memcpy(original_processes, processes, sizeof(Process) * count);

    // 동적으로 확장 가능한 프로세스 배열
    int max_processes = count * 20; // 충분한 공간 할당
    Process *all_processes = malloc(sizeof(Process) * max_processes);
    int total_process_count = 0;

    // 초기 프로세스들은 실제 arrival_time에 도착하므로 여기서 추가하지 않음
    // next_arrival을 초기 arrival_time으로 설정
    int *next_arrival = malloc(sizeof(int) * count);
    for (int i = 0; i < count; i++) {
        next_arrival[i] = original_processes[i].arrival_time;
    }

    DeadlineMissInfo *deadline_miss_info =
        malloc(sizeof(DeadlineMissInfo) * count * 10);
    config->deadline_miss_info_count = 0;

    int *waiting_q = malloc(sizeof(int) * max_processes);
    for (int i = 0; i < max_processes; i++) {
        waiting_q[i] = -1;
    }

    int time = 0;
    int idle_time = 0;

    print_utilization_analysis(original_processes, count, "EDF");

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

    while (time < max_time) {
        // 새로운 프로세스 인스턴스 생성 (초기 도착 + 주기적 도착)
        for (int i = 0; i < count; i++) {
            if (next_arrival[i] == time) {
                // 새로운 프로세스 인스턴스 생성
                Process new_process;
                memcpy(&new_process, &original_processes[i], sizeof(Process));

                // 새로운 인스턴스의 속성 설정 (PID는 원본과 동일하게 유지)
                new_process.pid = i; // 원본 프로세스와 동일한 PID 유지
                new_process.arrival_time = time;

                // 첫 번째 도착인지 주기적 도착인지 확인하여 deadline 설정
                if (time == original_processes[i].arrival_time) {
                    // 첫 번째 도착 - 원본 deadline 사용
                    new_process.deadline = original_processes[i].deadline;
                } else {
                    // 주기적 도착 - deadline 계산
                    int period_count =
                        (time - original_processes[i].arrival_time) /
                        original_processes[i].period;
                    new_process.deadline =
                        original_processes[i].deadline +
                        period_count * original_processes[i].period;
                }

                new_process.progress = 0;
                new_process.waiting_time_counter = 0;
                new_process.comp_time = 0;
                new_process.waiting_time = 0;
                new_process.turnaround_time = 0;
                new_process.missed_deadline = 0;

                // all_processes 배열에 추가
                all_processes[total_process_count] = new_process;

                // 다음 도착 시간 업데이트
                next_arrival[i] += original_processes[i].period;

                // 스케줄링 결정 (deadline 기준)
                if (!is_empty(&running_q)) {
                    int current_running = peek(&running_q);
                    if (all_processes[total_process_count].deadline <
                        all_processes[current_running].deadline) {
                        int preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, total_process_count);
                    } else {
                        // 단순 enqueue (나중에 일괄 정렬)
                        enqueue(&ready_q, total_process_count);
                    }
                } else {
                    // 단순 enqueue (나중에 일괄 정렬)
                    enqueue(&ready_q, total_process_count);
                }

                total_process_count++;
            }
        }

        // I/O 완료 처리
        for (int i = 0; i < total_process_count; i++) {
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 스케줄링 결정
                    if (!is_empty(&running_q)) {
                        int current_running = peek(&running_q);
                        if (all_processes[i].deadline <
                            all_processes[current_running].deadline) {
                            int preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                        } else {
                            enqueue(&ready_q, i);
                        }
                    } else {
                        enqueue(&ready_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }
        }

        // Ready queue를 deadline 기준으로 정렬 (CPU 할당 직전에 일괄 정렬)
        if (!is_empty(&ready_q)) {
            sort_queue(&ready_q, all_processes, SORT_BY_DEADLINE);
        }

        // CPU 스케줄링 (Ready → Running)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            int next_process = dequeue(&ready_q); // 이미 정렬되어 있으므로 첫
                                                  // 번째가 가장 빠른 deadline
            enqueue(&running_q, next_process);
        }

        // 프로세스 실행
        if (is_empty(&running_q)) {
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            int current_running = peek(&running_q);
            // Gantt 차트에는 실제 PID를 사용
            add_gantt_entry(&gantt, time, time + 1,
                            all_processes[current_running].pid, "RUN");

            all_processes[current_running].progress++;

            // 멀티 I/O 처리: 현재 진행도에서 I/O가 시작되는지 확인
            if (has_io_at_progress(&all_processes[current_running],
                                   all_processes[current_running].progress)) {
                int io_process = current_running;
                int io_burst = get_io_burst_at_progress(
                    &all_processes[io_process],
                    all_processes[io_process].progress);
                waiting_q[io_process] = io_burst + 1;
                dequeue(&running_q);
            }
            // 프로세스 완료 처리
            else if (all_processes[current_running].progress ==
                     all_processes[current_running].cpu_burst) {
                int finished = current_running;
                all_processes[finished].comp_time = time + 1;
                all_processes[finished].turnaround_time =
                    all_processes[finished].comp_time -
                    all_processes[finished].arrival_time;
                all_processes[finished].waiting_time =
                    all_processes[finished].waiting_time_counter;

                // 데드라인 미스 체크
                if (all_processes[finished].comp_time >
                    all_processes[finished].deadline) {
                    deadline_miss_info[config->deadline_miss_info_count].pid =
                        all_processes[finished].pid; // 실제 PID 사용
                    deadline_miss_info[config->deadline_miss_info_count]
                        .absolute_deadline = all_processes[finished].deadline;
                    deadline_miss_info[config->deadline_miss_info_count]
                        .completion_time = all_processes[finished].comp_time;

                    all_processes[finished].missed_deadline = 1;
                    config->deadline_miss_info_count++;
                    has_missed = 1;

                    printf("| %4d | P%-10d | %-16d | %-15d |\n", time + 1,
                           all_processes[finished].pid,
                           all_processes[finished].deadline,
                           all_processes[finished].comp_time);
                }

                dequeue(&running_q);
            }
        }

        // Ready queue의 프로세스들만 대기시간 증가
        if (!is_empty(&ready_q)) {
            for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
                 cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
                int pid = ready_q.data[i];
                all_processes[pid].waiting_time_counter++;
            }
        }

        // 디버깅용 ready queue 상태 출력
        // if (!is_empty(&ready_q)) {
        //     printf("Time %d Ready Queue (Deadline order): ", time);
        //     for (int i = ready_q.front, cnt = 0; cnt < ready_q.count;
        //          cnt++, i = (i + 1) % MAX_QUEUE_SIZE) {
        //         int pid = ready_q.data[i];
        //         printf("P%d(pr:%d) ", all_processes[pid].pid,
        //                all_processes[pid].period);
        //     }
        // }

        time++;
    }

    if (!has_missed) {
        printf("|                There is no deadline miss                |\n");
    }
    printf("+------+-------------+------------------+-----------------+\n\n");

    metrics->total_time = time;
    metrics->idle_time = idle_time;
    metrics->for_edf_rms_processes = all_processes;
    metrics->for_edf_rms_counter = total_process_count;
    printf("** Total Deadline Misses: %d **\n",
           config->deadline_miss_info_count);

    // 모든 프로세스 인스턴스를 포함하여 결과 출력
    display_scheduling_results(all_processes, total_process_count, &gantt, time,
                               idle_time, "Earliest Deadline First Multi-I/O");

    free(gantt.entries);
    free(original_processes);
    free(all_processes);
    free(next_arrival);
    free(deadline_miss_info);
    free(waiting_q);

    return metrics;
}
