#include "scheduler.h"
#include "evaluation.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

// FCFS 알고리즘 구현
Metrics *run_fcfs(Process *processes, int count, Config *config) {
    printf("\n\n[FCFS Scheduling]\n\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));
    // 프로세스 상태 초기화
    reset_processes(processes, count);

    // 간트차트 데이터 저장용
    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * 1000);
    gantt.count = 0;
    gantt.capacity = 1000;

    // 필요한 변수 초기화
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

    // 스케줄링 시뮬레이션
    while (completed < count) {
        // 도착한 프로세스를 ready 큐에 추가
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            // I/O 작업 중인 프로세스 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                }
            }
        }

        // CPU 할당
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);
            processes[pick].start = TRUE;
        }

        // 프로세스 실행 또는 IDLE 상태 처리
        if (is_empty(&running_q)) {
            // 간트차트에 IDLE 상태 기록
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
        } else {
            // 간트차트에 프로세스 실행 기록
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");

            if (processes[pick].start == TRUE) {
                processes[pick].start = FALSE;
            }

            processes[pick].progress++;

            // I/O 요청 처리
            if (processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            }
            // 프로세스 완료 처리
            else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                completed++;
            }
        }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("time: %d, idle_time: %d\n", time, idle_time);
    // 결과를 이미지와 같은 형태로 표시
    // 결과를 이미지와 같은 형태로 표시 (통합된 간트차트 사용)
    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "FCFS");

    // 또는 컬러 버전으로 표시 (터미널이 ANSI 컬러를 지원하는 경우)
    // display_colored_gantt_chart(&gantt);

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

// SJF(비선점) 알고리즘 구현
Metrics *run_sjf_np(Process *processes, int count, Config *config) {
    printf("\n\n[Non-Preemptive SJF Scheduling]\n\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    // 프로세스 상태 초기화
    reset_processes(processes, count);

    // 간트차트 데이터 저장용
    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * 1000);
    gantt.count = 0;
    gantt.capacity = 1000;

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
    // 스케줄링 시뮬레이션
    while (completed < count) {
        // 도착한 프로세스를 ready 큐에 추가
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                // SJF 기준으로 정렬하여 큐에 삽입
                if (is_empty(&ready_q)) {
                    enqueue(&ready_q, i);
                } else {
                    // 현재 ready_q에 있는 프로세스들을 CPU burst 순으로 정렬 후
                    // 삽입
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

                    // temp_q에서 ready_q로 다시 이동
                    while (!is_empty(&temp_q)) {
                        enqueue(&ready_q, dequeue(&temp_q));
                    }
                }
            }

            // I/O 작업 중인 프로세스 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 ready 큐에 삽입 (SJF 기준으로)
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

        // CPU 할당 (running_q가 비어있고 ready_q에 프로세스가 있을 때)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);

            // 처음 시작하는 프로세스의 경우
            if (processes[pick].progress == 0) {
                processes[pick].start = TRUE;
            }
        }

        // 프로세스 실행 또는 IDLE 상태 처리
        if (is_empty(&running_q)) {
            // 간트차트에 IDLE 상태 기록
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
            // printf("TIME %d ~ %d\t: IDLE\n", time, time + 1);
        } else {
            // 간트차트에 프로세스 실행 기록
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            // printf("TIME %d ~ %d\t: P[%d]\n", time, time + 1, pick);

            // 프로세스가 처음 시작될 때 응답 시간 계산
            if (processes[pick].start == TRUE) {
                processes[pick].start = FALSE;
            }

            // 프로세스 진행 상태 업데이트
            processes[pick].progress++;

            // I/O 요청 처리 (프로세스의 실행 시간이 io_start와 같을 때)
            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                // printf("I/O request from P[%d] at time %d\n", waiting,
                //        time + 1);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            }
            // 프로세스 완료 처리
            else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].turnaround_time -
                    processes[finished].cpu_burst;
                // printf("Process P[%d] completed at time %d\n", finished,
                //        time + 1);
                completed++;
            }
        }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("time: %d, idle_time: %d\n", time, idle_time);
    // 결과 표시
    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Non-Preemptive SJF");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_sjf_p(Process *processes, int count, Config *config) {
    printf("\n\n[Preemptive SJF Scheduling]\n\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    // 프로세스 상태 초기화
    reset_processes(processes, count);

    // 간트차트 데이터 저장용
    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * 1000);
    gantt.count = 0;
    gantt.capacity = 1000;

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
    // 스케줄링 시뮬레이션
    while (completed < count) {
        // 도착한 프로세스를 ready 큐에 추가하고 선점 검사
        for (int i = 0; i < count; i++) {
            // I/O 작업 중인 프로세스 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 ready 큐 또는 running 큐에 삽입 (선점형 SJF)
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
                            // printf("Process P[%d] preempted P[%d] at time
                            // %d\n", i, preempted, time);
                        } else {
                            // 선점 없이 ready 큐에 삽입
                            if (is_empty(&ready_q)) {
                                enqueue(&ready_q, i);
                            } else {
                                // SJF 기준으로 정렬하여 삽입
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
                        // running 큐가 비어있으면 바로 실행
                        enqueue(&running_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }

            // 새로 도착한 프로세스 처리
            if (processes[i].arrival_time == time) {
                // 현재 실행 중인 프로세스가 있으면 선점 검사
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);
                    int remaining_current = processes[current].cpu_burst -
                                            processes[current].progress;
                    int remaining_i = processes[i].cpu_burst;

                    if (remaining_i < remaining_current) {
                        // 선점 발생
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                        // printf("Process P[%d] preempted P[%d] at time %d\n",
                        // i, preempted, time);
                    } else {
                        // 선점 없이 ready 큐에 삽입
                        if (is_empty(&ready_q)) {
                            enqueue(&ready_q, i);
                        } else {
                            // SJF 기준으로 정렬하여 삽입
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
                    // running 큐와 ready 큐가 모두 비어있으면 바로 실행
                    enqueue(&running_q, i);
                } else {
                    // ready 큐에만 프로세스가 있는 경우
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

        // CPU 할당 (running_q가 비어있고 ready_q에 프로세스가 있을 때)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);

            // 처음 시작하는 프로세스의 경우
            if (processes[pick].progress == 0) {
                processes[pick].start = TRUE;
            }
        }

        // 프로세스 실행 또는 IDLE 상태 처리
        if (is_empty(&running_q)) {
            // 간트차트에 IDLE 상태 기록
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
            // printf("TIME %d ~ %d\t: IDLE\n", time, time + 1);
        } else {
            pick = peek(&running_q);
            // 간트차트에 프로세스 실행 기록
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            // printf("TIME %d ~ %d\t: P[%d]\n", time, time + 1, pick);

            if (processes[pick].start == TRUE) {
                processes[pick].start = FALSE;
            }

            // 프로세스 진행 상태 업데이트
            processes[pick].progress++;
            processes[pick].remaining_time =
                processes[pick].cpu_burst - processes[pick].progress;

            // I/O 요청 처리 (프로세스의 실행 시간이 io_start와 같을 때)
            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                // printf("I/O request from P[%d] at time %d\n", waiting,
                //        time + 1);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            }
            // 프로세스 완료 처리
            else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].turnaround_time -
                    processes[finished].cpu_burst;
                // printf("Process P[%d] completed at time %d\n", finished,
                //        time + 1);
                completed++;
            }
        }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("time: %d, idle_time: %d\n", time, idle_time);
    // 결과 표시
    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Preemptive SJF");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_priority_np(Process *processes, int count, Config *config) {
    printf("\n\n[Non-Preemptive Priority Scheduling]\n\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    // 프로세스 상태 초기화
    reset_processes(processes, count);

    // 간트차트 데이터 저장용
    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * 1000);
    gantt.count = 0;
    gantt.capacity = 1000;

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

    // 스케줄링 시뮬레이션
    while (completed < count) {
        // 도착한 프로세스를 ready 큐에 추가
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                // 우선순위 기준으로 정렬하여 큐에 삽입
                if (is_empty(&ready_q)) {
                    enqueue(&ready_q, i);
                } else {
                    // 현재 ready_q에 있는 프로세스들을 우선순위 순으로 정렬하며
                    // 삽입
                    int inserted = 0;
                    while (!is_empty(&ready_q)) {
                        int current = peek(&ready_q);
                        // 우선순위 값이 낮을수록 높은 우선순위
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

                    // temp_q에서 ready_q로 다시 이동
                    while (!is_empty(&temp_q)) {
                        enqueue(&ready_q, dequeue(&temp_q));
                    }
                }
            }

            // I/O 작업 중인 프로세스 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 ready 큐에 삽입 (우선순위 기준으로)
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

        // CPU 할당 (running_q가 비어있고 ready_q에 프로세스가 있을 때)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);

            // 처음 시작하는 프로세스의 경우
            if (processes[pick].progress == 0) {
                processes[pick].start = TRUE;
            }
        }

        // 프로세스 실행 또는 IDLE 상태 처리
        if (is_empty(&running_q)) {
            // 간트차트에 IDLE 상태 기록
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
            // printf("TIME %d ~ %d\t: IDLE\n", time, time + 1);
        } else {
            // 간트차트에 프로세스 실행 기록
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            // printf("TIME %d ~ %d\t: P[%d]\n", time, time + 1, pick);

            // 프로세스가 처음 시작될 때 응답 시간 계산
            if (processes[pick].start == TRUE) {
                processes[pick].start = FALSE;
            }

            // 프로세스 진행 상태 업데이트
            processes[pick].progress++;

            // I/O 요청 처리 (프로세스의 실행 시간이 io_start와 같을 때)
            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                // printf("I/O request from P[%d] at time %d\n", waiting,
                //        time + 1);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            }
            // 프로세스 완료 처리
            else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].turnaround_time -
                    processes[finished].cpu_burst;
                // printf("Process P[%d] completed at time %d\n", finished,
                //        time + 1);
                completed++;
            }
        }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("time: %d, idle_time: %d\n", time, idle_time);
    // 결과 표시
    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Non-Preemptive Priority");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_priority_p(Process *processes, int count, Config *config) {
    printf("\n\n[Preemptive Priority Scheduling]\n\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    // 프로세스 상태 초기화
    reset_processes(processes, count);

    // 간트차트 데이터 저장용
    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * 1000);
    gantt.count = 0;
    gantt.capacity = 1000;

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

    // 스케줄링 시뮬레이션
    while (completed < count) {
        // 도착한 프로세스를 ready 큐에 추가하고 선점 검사
        for (int i = 0; i < count; i++) {
            // I/O 작업 중인 프로세스 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    // I/O 완료 후 ready 큐 또는 running 큐에 삽입 (선점형
                    // 우선순위)
                    if (!is_empty(&running_q)) {
                        int current = peek(&running_q);

                        if (processes[i].priority <
                            processes[current].priority) {
                            // 선점 발생
                            preempted = dequeue(&running_q);
                            enqueue(&ready_q, preempted);
                            enqueue(&running_q, i);
                            // printf("Process P[%d] preempted P[%d] at time
                            // %d\n", i, preempted, time);
                        } else {
                            // 선점 없이 ready 큐에 삽입
                            if (is_empty(&ready_q)) {
                                enqueue(&ready_q, i);
                            } else {
                                // 우선순위 기준으로 정렬하여 삽입
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
                        // running 큐가 비어있으면 바로 실행
                        enqueue(&running_q, i);
                    }
                    waiting_q[i] = -1;
                }
            }

            // 새로 도착한 프로세스 처리
            if (processes[i].arrival_time == time) {
                // 현재 실행 중인 프로세스가 있으면 선점 검사
                if (!is_empty(&running_q)) {
                    int current = peek(&running_q);

                    if (processes[i].priority < processes[current].priority) {
                        // 선점 발생
                        preempted = dequeue(&running_q);
                        enqueue(&ready_q, preempted);
                        enqueue(&running_q, i);
                        // printf("Process P[%d] preempted P[%d] at time %d\n",
                        // i, preempted, time);
                    } else {
                        // 선점 없이 ready 큐에 삽입
                        if (is_empty(&ready_q)) {
                            enqueue(&ready_q, i);
                        } else {
                            // 우선순위 기준으로 정렬하여 삽입
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
                    // running 큐와 ready 큐가 모두 비어있으면 바로 실행
                    enqueue(&running_q, i);
                } else {
                    // ready 큐에만 프로세스가 있는 경우
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

        // CPU 할당 (running_q가 비어있고 ready_q에 프로세스가 있을 때)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);

            // 처음 시작하는 프로세스의 경우
            if (processes[pick].progress == 0) {
                processes[pick].start = TRUE;
            }
        }

        // 프로세스 실행 또는 IDLE 상태 처리
        if (is_empty(&running_q)) {
            // 간트차트에 IDLE 상태 기록
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
            // printf("TIME %d ~ %d\t: IDLE\n", time, time + 1);
        } else {
            pick = peek(&running_q);
            // 간트차트에 프로세스 실행 기록
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            // printf("TIME %d ~ %d\t: P[%d]\n", time, time + 1, pick);

            // 프로세스가 처음 시작될 때 응답 시간 계산
            if (processes[pick].start == TRUE) {
                processes[pick].start = FALSE;
            }

            // 프로세스 진행 상태 업데이트
            processes[pick].progress++;

            // I/O 요청 처리 (프로세스의 실행 시간이 io_start와 같을 때)
            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                // printf("I/O request from P[%d] at time %d\n", waiting, time +
                // 1);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
            }
            // 프로세스 완료 처리
            else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].turnaround_time -
                    processes[finished].cpu_burst;
                // printf("Process P[%d] completed at time %d\n", finished, time
                // + 1);
                completed++;
            }
        }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("time: %d, idle_time: %d\n", time, idle_time);
    // 결과 표시
    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Preemptive Priority");

    free(gantt.entries);
    free(waiting_q);

    return metrics;
}

Metrics *run_rr(Process *processes, int count, Config *config) {
    printf("\n\n[Round Robin Scheduling]\n\n");

    Metrics *metrics;
    metrics = malloc(sizeof(Metrics));

    // 프로세스 상태 초기화
    reset_processes(processes, count);

    // 간트차트 데이터 저장용
    GanttChart gantt;
    gantt.entries = malloc(sizeof(GanttEntry) * 1000);
    gantt.count = 0;
    gantt.capacity = 1000;

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

    // 타임 퀀텀 값 설정
    int quantum = config->time_quantum;
    printf("Time Quantum: %d\n", quantum);

    // 스케줄링 시뮬레이션
    while (completed < count) {
        // 도착한 프로세스를 ready 큐에 추가
        for (int i = 0; i < count; i++) {
            if (processes[i].arrival_time == time) {
                enqueue(&ready_q, i);
            }

            // I/O 작업 중인 프로세스 처리
            if (waiting_q[i] > 0) {
                waiting_q[i]--;
                if (waiting_q[i] == 0) {
                    enqueue(&ready_q, i);
                    waiting_q[i] = -1;
                    time_quantum[i] = 0; // 타임 퀀텀 초기화
                }
            }
        }

        // CPU 할당 (running_q가 비어있고 ready_q에 프로세스가 있을 때)
        if (is_empty(&running_q) && !is_empty(&ready_q)) {
            pick = dequeue(&ready_q);
            enqueue(&running_q, pick);

            // 처음 시작하는 프로세스의 경우
            if (processes[pick].progress == 0) {
                processes[pick].start = TRUE;
            }
        }

        // 프로세스 실행 또는 IDLE 상태 처리
        if (is_empty(&running_q)) {
            // 간트차트에 IDLE 상태 기록
            add_gantt_entry(&gantt, time, time + 1, -1, "IDLE");
            idle_time++;
            // printf("TIME %d ~ %d\t: IDLE\n", time, time + 1);
        } else {
            pick = peek(&running_q);
            // 간트차트에 프로세스 실행 기록
            add_gantt_entry(&gantt, time, time + 1, pick, "RUN");
            // printf("TIME %d ~ %d\t: P[%d]\n", time, time + 1, pick);

            // 프로세스가 처음 시작될 때 응답 시간 계산
            if (processes[pick].start == TRUE) {
                processes[pick].start = FALSE;
            }

            // 프로세스 진행 상태 업데이트
            processes[pick].progress++;
            time_quantum[pick]++;

            // I/O 요청 처리 (프로세스의 실행 시간이 io_start와 같을 때)
            if (processes[pick].io_burst > 0 &&
                processes[pick].progress == processes[pick].io_start) {
                int waiting = dequeue(&running_q);
                // printf("I/O request from P[%d] at time %d\n", waiting,
                //        time + 1);
                waiting_q[waiting] = processes[waiting].io_burst + 1;
                time_quantum[waiting] = 0; // 타임 퀀텀 초기화
            }
            // 프로세스 완료 처리
            else if (processes[pick].progress == processes[pick].cpu_burst) {
                int finished = dequeue(&running_q);
                processes[finished].comp_time = time + 1;
                processes[finished].turnaround_time =
                    processes[finished].comp_time -
                    processes[finished].arrival_time;
                processes[finished].waiting_time =
                    processes[finished].turnaround_time -
                    processes[finished].cpu_burst;
                // printf("Process P[%d] completed at time %d\n", finished,
                //        time + 1);
                completed++;
                time_quantum[finished] = 0; // 타임 퀀텀 초기화
            }
            // 타임 퀀텀 만료 시 프로세스 교체
            else if (time_quantum[pick] == quantum) {
                int rotated = dequeue(&running_q);
                // printf("Time quantum expired for P[%d] at time %d\n",
                // rotated, time + 1);
                enqueue(&ready_q, rotated);
                time_quantum[rotated] = 0; // 타임 퀀텀 초기화
            }
        }

        time++;
    }

    metrics->total_time = time;
    metrics->idle_time = idle_time;

    // printf("time: %d, idle_time: %d\n", time, idle_time);
    // 결과 표시
    display_scheduling_results(processes, count, &gantt, time, idle_time,
                               "Round Robin");

    free(gantt.entries);
    free(waiting_q);
    free(time_quantum);

    return metrics;
}
