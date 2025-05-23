// evaluation.c에 추가할 함수들

#include "evaluation.h"
#include "process.h"
#include "queue.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 간트차트 엔트리 추가 함수 (추적을 위해서)
void add_gantt_entry(GanttChart *gantt, int start, int end, int pid,
                     const char *status) {
    if (gantt->count < gantt->capacity) {
        gantt->entries[gantt->count].time_start = start;
        gantt->entries[gantt->count].time_end = end;
        gantt->entries[gantt->count].process_id = pid;
        strcpy(gantt->entries[gantt->count].status, status);
        gantt->count++;
    }
}

// 알고리즘 비교 함수
void compare_algorithms(Process *processes, int count, Config *config) {
    printf("\n===== Algorithm Comparison =====\n");

    // 성능 지표를 저장할 구조체 배열
    typedef struct {
        char name[32];
        float avg_wait_time;
        float avg_turnaround_time;
        float cpu_utilization;
        float throughput;
        int total_time;
    } AlgorithmMetrics;

    AlgorithmMetrics metrics[6]; // 6개 알고리즘

    // 원본 프로세스 배열 복사
    Process *copy_processes = malloc(sizeof(Process) * count);
    for (int i = 0; i < count; i++) {
        copy_processes[i] = processes[i];
    }

    // 1. FCFS
    reset_processes(processes, count);
    Metrics *metrics_temp = run_fcfs(processes, count, config);

    int total_waiting = 0;
    int total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    strcpy(metrics[0].name, "FCFS");
    metrics[0].avg_wait_time = (float)total_waiting / count;
    metrics[0].avg_turnaround_time = (float)total_turnaround / count;
    metrics[0].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[0].throughput = ((float)count / metrics_temp->total_time) * 100.0;
    metrics[0].total_time = metrics_temp->total_time;

    // 2. Non-preemptive SJF
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_sjf_np(processes, count, config);

    total_waiting = 0;
    total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    strcpy(metrics[1].name, "Non-Preemptive SJF");
    metrics[1].avg_wait_time = (float)total_waiting / count;
    metrics[1].avg_turnaround_time = (float)total_turnaround / count;
    metrics[1].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[1].throughput = ((float)count / metrics_temp->total_time) * 100.0;
    metrics[1].total_time = metrics_temp->total_time;
    // printf("cpu_utilization: %f, idle_time: %d\n",
    // metrics[1].cpu_utilization,
    //        metrics_temp->idle_time);

    // 3. Preemptive SJF
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_sjf_p(processes, count, config);

    total_waiting = 0;
    total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    strcpy(metrics[2].name, "Preemptive SJF");
    metrics[2].avg_wait_time = (float)total_waiting / count;
    metrics[2].avg_turnaround_time = (float)total_turnaround / count;
    metrics[2].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[2].throughput = ((float)count / metrics_temp->total_time) * 100.0;
    metrics[2].total_time = metrics_temp->total_time;
    // printf("cpu_utilization: %f, idle_time: %d\n",
    // metrics[2].cpu_utilization,
    //        metrics_temp->idle_time);

    // 4. Non-preemptive Priority
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_priority_np(processes, count, config);

    total_waiting = 0;
    total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    strcpy(metrics[3].name, "Non-Preemptive Priority");
    metrics[3].avg_wait_time = (float)total_waiting / count;
    metrics[3].avg_turnaround_time = (float)total_turnaround / count;
    metrics[3].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[3].throughput = ((float)count / metrics_temp->total_time) * 100.0;
    metrics[3].total_time = metrics_temp->total_time;
    // printf("cpu_utilization: %f, idle_time: %d\n",
    // metrics[3].cpu_utilization,
    //        metrics_temp->idle_time);

    // 5. Preemptive Priority
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_priority_p(processes, count, config);

    total_waiting = 0;
    total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    strcpy(metrics[4].name, "Preemptive Priority");
    metrics[4].avg_wait_time = (float)total_waiting / count;
    metrics[4].avg_turnaround_time = (float)total_turnaround / count;
    metrics[4].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[4].throughput = ((float)count / metrics_temp->total_time) * 100.0;
    metrics[4].total_time = metrics_temp->total_time;
    // printf("cpu_utilization: %f, idle_time: %d\n",
    // metrics[4].cpu_utilization,
    //        metrics_temp->idle_time);

    // 6. Round Robin
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_rr(processes, count, config);

    total_waiting = 0;
    total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    strcpy(metrics[5].name, "Round Robin");
    metrics[5].avg_wait_time = (float)total_waiting / count;
    metrics[5].avg_turnaround_time = (float)total_turnaround / count;
    metrics[5].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[5].throughput = ((float)count / metrics_temp->total_time) * 100.0;
    metrics[5].total_time = metrics_temp->total_time;
    // printf("cpu_utilization: %f, idle_time: %d\n",
    // metrics[5].cpu_utilization,
    //        metrics_temp->idle_time);

    // 결과 비교 출력
    printf("\n===== CPU Scheduling Algorithm Comparison =====\n");
    printf("+----------------------+---------------+------------------+--------"
           "------+------------+\n");
    printf("| Algorithm            | Avg Wait Time | Avg Turnaround   | CPU "
           "Util (%%) | Throughput |\n");
    printf("+----------------------+---------------+------------------+--------"
           "------+------------+\n");

    for (int i = 0; i < 6; i++) {
        char short_name[20];
        if (strcmp(metrics[i].name, "Non-Preemptive SJF") == 0) {
            strcpy(short_name, "NP SJF");
        } else if (strcmp(metrics[i].name, "Preemptive SJF") == 0) {
            strcpy(short_name, "P SJF");
        } else if (strcmp(metrics[i].name, "Non-Preemptive Priority") == 0) {
            strcpy(short_name, "NP Priority");
        } else if (strcmp(metrics[i].name, "Preemptive Priority") == 0) {
            strcpy(short_name, "P Priority");
        } else {
            strcpy(short_name, metrics[i].name);
        }

        printf("| %-20s | %13.2f | %16.2f | %11.2f%% | %10.4f |\n", short_name,
               metrics[i].avg_wait_time, metrics[i].avg_turnaround_time,
               metrics[i].cpu_utilization, metrics[i].throughput);
    }

    printf("+----------------------+---------------+------------------+--------"
           "------+------------+\n");

    // 결과를 파일로 저장
    FILE *fp = fopen("test/scheduling_comparison_report.txt", "w");
    if (fp) {
        // 시뮬레이션 설정 정보 저장
        fprintf(fp,
                "===== CPU Scheduling Simulator Comparison Report =====\n\n");
        fprintf(fp, "* Simulation Configuration:\n");
        fprintf(fp, " - Number of processes: %d\n", count);
        fprintf(fp, " - Time quantum (RR): %d\n", config->time_quantum);
        fprintf(fp, " - Priority range: 1-5 (1 being highest)\n");
        fprintf(fp, " - CPU burst time range: 2-10\n");
        fprintf(
            fp,
            " - I/O burst time range: 1-5 (when cpu burst is higher than 2)\n");

        // 성능 지표 요약
        fprintf(fp, "* Performance Metrics Summary:\n");
        fprintf(fp,
                " +----------------------+---------------+------------------"
                "+--------------+------------+\n");
        fprintf(fp,
                " | Algorithm            | Avg Wait Time | Avg Turnaround   "
                "| CPU Util (%%) | Throughput |\n");
        fprintf(fp,
                " +----------------------+---------------+------------------"
                "+--------------+------------+\n");

        for (int i = 0; i < 6; i++) {
            char short_name[20];
            if (strcmp(metrics[i].name, "Non-Preemptive SJF") == 0) {
                strcpy(short_name, "NP SJF");
            } else if (strcmp(metrics[i].name, "Preemptive SJF") == 0) {
                strcpy(short_name, "P SJF");
            } else if (strcmp(metrics[i].name, "Non-Preemptive Priority") ==
                       0) {
                strcpy(short_name, "NP Priority");
            } else if (strcmp(metrics[i].name, "Preemptive Priority") == 0) {
                strcpy(short_name, "P Priority");
            } else {
                strcpy(short_name, metrics[i].name);
            }

            fprintf(fp, " | %-20s | %13.2f | %16.2f | %11.2f%% | %10.4f |\n",
                    short_name, metrics[i].avg_wait_time,
                    metrics[i].avg_turnaround_time, metrics[i].cpu_utilization,
                    metrics[i].throughput);
        }

        fprintf(fp,
                " +----------------------+---------------+------------------"
                "+--------------+------------+\n\n\n");

        // 알고리즘 분석
        fprintf(fp, "* Algorithm Analysis:\n");

        // 평균 대기 시간이 가장 짧은 알고리즘 찾기
        int min_wait_idx = 0;
        for (int i = 1; i < 6; i++) {
            if (metrics[i].avg_wait_time <
                metrics[min_wait_idx].avg_wait_time) {
                min_wait_idx = i;
            }
        }
        fprintf(fp, " - Minimum Average Waiting Time: %s (%.2f)\n",
                metrics[min_wait_idx].name,
                metrics[min_wait_idx].avg_wait_time);

        // 평균 반환 시간이 가장 짧은 알고리즘 찾기
        int min_turnaround_idx = 0;
        for (int i = 1; i < 6; i++) {
            if (metrics[i].avg_turnaround_time <
                metrics[min_turnaround_idx].avg_turnaround_time) {
                min_turnaround_idx = i;
            }
        }
        fprintf(fp, " - Minimum Average Turnaround Time: %s (%.2f)\n",
                metrics[min_turnaround_idx].name,
                metrics[min_turnaround_idx].avg_turnaround_time);

        // CPU 사용률이 가장 높은 알고리즘 찾기
        int max_cpu_util_idx = 0;
        for (int i = 1; i < 6; i++) {
            if (metrics[i].cpu_utilization >
                metrics[max_cpu_util_idx].cpu_utilization) {
                max_cpu_util_idx = i;
            }
        }
        fprintf(fp, " - Maximum CPU Utilization: %s (%.2f%%)\n",
                metrics[max_cpu_util_idx].name,
                metrics[max_cpu_util_idx].cpu_utilization);

        // 처리량이 가장 높은 알고리즘 찾기
        int max_throughput_idx = 0;
        for (int i = 1; i < 6; i++) {
            if (metrics[i].throughput >
                metrics[max_throughput_idx].throughput) {
                max_throughput_idx = i;
            }
        }
        fprintf(fp, " - Maximum Throughput: %s (%.4f)\n\n\n",
                metrics[max_throughput_idx].name,
                metrics[max_throughput_idx].throughput);

        // 결론
        fprintf(fp, "* Conclusion:\n");

        fprintf(fp,
                " - For interactive systems where quick user response is "
                "critical, %s "
                "is ideal due to its low average waiting time of %.2f ms.\n",
                metrics[min_wait_idx].name,
                metrics[min_wait_idx].avg_wait_time);

        fprintf(
            fp,
            " - For time-sensitive applications requiring fast task "
            "completion, "
            "%s performs best with the lowest turnaround time of %.2f ms.\n",
            metrics[min_turnaround_idx].name,
            metrics[min_turnaround_idx].avg_turnaround_time);

        fprintf(
            fp,
            " - For real-time systems that demand high CPU utilization, %s is "
            "most suitable with a utilization of %.2f%%.\n",
            metrics[max_cpu_util_idx].name,
            metrics[max_cpu_util_idx].cpu_utilization);

        fprintf(
            fp,
            " - For batch systems where throughput is important, %s would be "
            "most appropriate with a throughput of %.4f.\n\n\n\n",
            metrics[max_throughput_idx].name,
            metrics[max_throughput_idx].throughput);

        fclose(fp);
        printf("Comparison report generated and saved as "
               "'scheduling_comparison_report.txt'\n");
    } else {
        printf("\nError: Could not create comparison report file.\n");
    }

    // 원본 프로세스 상태 복원
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }

    free(copy_processes);
}

void display_performance_summary(Process *processes, int count, int total_time,
                                 int idle_time) {
    int total_waiting = 0;
    int total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        processes[i].turnaround_time =
            processes[i].comp_time - processes[i].arrival_time;
        processes[i].waiting_time =
            processes[i].turnaround_time - processes[i].cpu_burst;

        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    float avg_waiting = (float)total_waiting / count;
    float avg_turnaround = (float)total_turnaround / count;

    printf("\nAverage Waiting Time    : %.2f\n", avg_waiting);
    printf("Average Turnaround Time : %.2f\n", avg_turnaround);
}

// 프로세스 상세 정보를 테이블 형태로 표시
void display_process_table(Process *processes, int count) {
    printf(
        "+-----+------------+-------------+----------+----------+------------+-"
        "-------------+-------------+----------------+\n");
    printf(
        "| PID | Burst Time | Arrive Time | Priority | IO Start | IO Burst   | "
        "Return Time  | Waiting Time| Turnaround Time|\n");
    printf(
        "+-----+------------+-------------+----------+----------+------------+-"
        "-------------+-------------+----------------+\n");

    // 각 프로세스 정보 출력
    for (int i = 0; i < count; i++) {

        // I/O 정보 표시 (없으면 "None" 출력)
        char io_start_str[10] = "None";
        char io_burst_str[10] = "None";

        if (processes[i].io_burst > 0) {
            sprintf(io_start_str, "%d", processes[i].io_start);
            sprintf(io_burst_str, "%d", processes[i].io_burst);
        }

        printf("| P%-2d |     %-6d |      %-6d |    %-5d | %-8s | %-10s |      "
               " %-6d |      %-6d |       %-8d |\n",
               processes[i].pid, processes[i].cpu_burst,
               processes[i].arrival_time, processes[i].priority, io_start_str,
               io_burst_str, processes[i].comp_time, processes[i].waiting_time,
               processes[i].turnaround_time);

        printf("+-----+------------+-------------+----------+----------+-------"
               "-----+--------------+-------------+------------"
               "----+\n");
    }
}

// 간트차트 데이터를 통합하는 함수 (연속된 같은 프로세스를 하나로 합치기)
GanttChart *consolidate_gantt_chart(GanttChart *original) {
    if (original->count == 0)
        return original;

    GanttChart *consolidated = malloc(sizeof(GanttChart));
    consolidated->entries = malloc(sizeof(GanttEntry) * original->count);
    consolidated->count = 0;
    consolidated->capacity = original->count;

    // 첫 번째 엔트리 추가
    consolidated->entries[0] = original->entries[0];
    consolidated->count = 1;

    // 나머지 엔트리들을 확인하면서 같은 프로세스면 합치기
    for (int i = 1; i < original->count; i++) {
        int last_idx = consolidated->count - 1;

        // 같은 프로세스가 연속으로 실행되는 경우
        if (consolidated->entries[last_idx].process_id ==
            original->entries[i].process_id) {
            // 끝 시간만 업데이트
            consolidated->entries[last_idx].time_end =
                original->entries[i].time_end;
        } else {
            // 다른 프로세스인 경우 새로운 엔트리 추가
            consolidated->entries[consolidated->count] = original->entries[i];
            consolidated->count++;
        }
    }

    return consolidated;
}

void display_gantt_chart(GanttChart *gantt, const char *algorithm_name) {
    GanttChart *consolidated = consolidate_gantt_chart(gantt);
    int i, j;

    printf(" ");
    for (i = 0; i < consolidated->count; i++) {
        int duration = consolidated->entries[i].time_end -
                       consolidated->entries[i].time_start;
        for (j = 0; j < duration; j++) {
            printf("--");
        }
        printf(" ");
    }
    printf("\n|");

    for (i = 0; i < consolidated->count; i++) {
        int duration = consolidated->entries[i].time_end -
                       consolidated->entries[i].time_start;

        if (consolidated->entries[i].process_id == -1) {
            // IDLE 처리
            for (j = 0; j < duration - 1; j++) {
                printf(" ");
            }
            printf("ID");
            for (j = 0; j < duration - 1; j++) {
                printf(" ");
            }
        } else {
            // 프로세스 처리
            char pid_str[10];
            sprintf(pid_str, "P%d", consolidated->entries[i].process_id);

            for (j = 0; j < duration - 1; j++) {
                printf(" ");
            }
            printf("%s", pid_str);
            for (j = 0; j < duration - 1; j++) {
                printf(" ");
            }
        }
        printf("|");
    }
    printf("\n ");

    for (i = 0; i < consolidated->count; i++) {
        int duration = consolidated->entries[i].time_end -
                       consolidated->entries[i].time_start;
        for (j = 0; j < duration; j++) {
            printf("--");
        }
        printf(" ");
    }
    printf("\n");

    printf("0");
    for (i = 0; i < consolidated->count; i++) {
        int duration = consolidated->entries[i].time_end -
                       consolidated->entries[i].time_start;
        int end_time = consolidated->entries[i].time_end;

        for (j = 0; j < duration; j++) {
            printf("  ");
        }

        // 두 자리 숫자일 때 백스페이스
        if (end_time > 9) {
            printf("\b");
        }
        printf("%d", end_time);
    }
    printf("\n");

    free(consolidated->entries);
    free(consolidated);
}
// 새로운 결과 표시 함수
void display_scheduling_results(Process *processes, int count,
                                GanttChart *gantt, int total_time,
                                int idle_time, const char *algorithm_name) {
    // 간트차트 표시
    display_gantt_chart(gantt, algorithm_name);

    // 성능 지표 표시
    display_performance_summary(processes, count, total_time, idle_time);

    // 프로세스 상세 정보 테이블 표시
    display_process_table(processes, count);
}
