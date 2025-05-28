
#include "evaluation.h"
#include "process.h"
#include "queue.h"
#include "scheduler.h"
#include <dirent.h> // DIR, struct dirent, opendir, readdir, closedir
#include <limits.h> // INT_MAX 등의 상수 정의
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // 파일 및 디렉토리 상태 확인

void print_thin_emphasized_header(const char *title, int width) {
    int title_len = strlen(title);
    int left_padding = (width - title_len - 2) / 2;
    int right_padding = width - title_len - 2 - left_padding;

    printf("▓");
    for (int i = 0; i < left_padding; i++)
        printf("▓");
    printf(" %s ", title);
    for (int i = 0; i < right_padding; i++)
        printf("▓");
    printf("▓\n");
}

void print_section_divider(const char *title, int width) {
    printf("\n┌─── %s ", title);
    for (int i = 0; i < width - strlen(title) - 1; i++)
        printf("─");
    printf("┐\n");
}

void print_section_end(int width) {
    printf("└");
    for (int i = 0; i < width; i++)
        printf("─");
    printf("┘\n");
}

void print_emphasized_header(const char *title, int width) {
    int title_len = strlen(title);
    int padding = (width - title_len - 2) / 2;
    int remaining = width - title_len - 2 - padding;

    printf("╔");
    for (int i = 0; i < width; i++)
        printf("═");
    printf("╗\n");

    printf("║");
    for (int i = 0; i < padding; i++)
        printf(" ");
    printf(" %s ", title);
    for (int i = 0; i < remaining; i++)
        printf(" ");
    printf("║\n");

    printf("╚");
    for (int i = 0; i < width; i++)
        printf("═");
    printf("╝\n");
}

// 간트차트 엔트리 추가 함수
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

void compare_algorithms(Process *processes, int count, Config *config) {
    print_emphasized_header("Algorithm Comparison", 150);

    AlgorithmMetrics metrics[9];

    Process *copy_processes = malloc(sizeof(Process) * count);
    for (int i = 0; i < count; i++) {
        copy_processes[i] = processes[i];
    } // 이젠 딱히 필요없음 -> 이미 scheduling.c에서 해줌 (근데 혹시 모르니까)

    // 1. FCFS
    reset_processes(processes, count);
    Metrics *metrics_temp = run_fcfs(processes, count);

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
    metrics[0].throughput = ((float)count / metrics_temp->total_time) *
                            100.0; // 단위 시간당 처리하는 프로세스 양
    metrics[0].total_time = metrics_temp->total_time;
    metrics[0].missed_deadlines = 0;

    // 2. Non-preemptive SJF
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_sjf_np(processes, count);

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
    metrics[1].missed_deadlines = 0;

    // 3. Preemptive SJF
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_sjf_p(processes, count);

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
    metrics[2].missed_deadlines = 0;

    // 4. Non-preemptive Priority
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_priority_np(processes, count);

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
    metrics[3].missed_deadlines = 0;

    // 5. Preemptive Priority
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_priority_p(processes, count);

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
    metrics[4].missed_deadlines = 0;

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
    metrics[5].missed_deadlines = 0;

    // 7. Priority with Aging
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_priority_with_aging(processes, count);

    total_waiting = 0;
    total_turnaround = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    strcpy(metrics[6].name, "Priority with Aging");
    metrics[6].avg_wait_time = (float)total_waiting / count;
    metrics[6].avg_turnaround_time = (float)total_turnaround / count;
    metrics[6].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[6].throughput = ((float)count / metrics_temp->total_time) * 100.0;
    metrics[6].total_time = metrics_temp->total_time;
    metrics[6].missed_deadlines = 0;

    // 8. EDF
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_edf(processes, count, config);

    total_waiting = 0;
    int edf_completed_processes = 0;
    int edf_missed_deadlines = config->deadline_miss_info_count;

    for (int i = 0; i < count; i++) {
        if (processes[i].comp_time > 0) {
            total_waiting += processes[i].waiting_time;
            edf_completed_processes++;
        }
    }

    strcpy(metrics[7].name, "EDF");
    metrics[7].avg_wait_time =
        edf_completed_processes > 0
            ? (float)total_waiting / edf_completed_processes
            : 0.0;
    metrics[7].avg_turnaround_time =
        -1.0; // EDF는 turnaround time을 계산하지 않음
    metrics[7].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[7].throughput =
        ((float)edf_completed_processes / metrics_temp->total_time) * 100.0;
    metrics[7].total_time = metrics_temp->total_time;
    metrics[7].missed_deadlines = edf_missed_deadlines;

    // 9. RMS
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_rms(processes, count, config);

    total_waiting = 0;
    int rms_completed_processes = 0;
    int rms_missed_deadlines = config->deadline_miss_info_count;

    for (int i = 0; i < count; i++) {
        if (processes[i].comp_time > 0) {
            total_waiting += processes[i].waiting_time;
            rms_completed_processes++;
        }
    }

    strcpy(metrics[8].name, "RMS");
    metrics[8].avg_wait_time =
        rms_completed_processes > 0
            ? (float)total_waiting / rms_completed_processes
            : 0.0;
    metrics[8].avg_turnaround_time =
        -1.0; // RMS는 turnaround time을 계산하지 않음
    metrics[8].cpu_utilization =
        ((float)(metrics_temp->total_time - metrics_temp->idle_time) /
         metrics_temp->total_time) *
        100.0;
    metrics[8].throughput =
        ((float)rms_completed_processes / metrics_temp->total_time) * 100.0;
    metrics[8].total_time = metrics_temp->total_time;
    metrics[8].missed_deadlines = rms_missed_deadlines;

    printf("\n\n");
    print_thin_emphasized_header("CPU Scheduling Algorithm Comparison", 115);
    printf("\n");
    printf(
        "+----------------------+---------------+------------------+----------"
        "----+------------+------------------+\n");
    printf("| Algorithm            | Avg Wait Time | Avg Turnaround   | CPU "
           "Util (%%) | Throughput | Missed Deadlines |\n");
    printf(
        "+----------------------+---------------+------------------+----------"
        "----+------------+------------------+\n");

    for (int i = 0; i < 9; i++) {
        char short_name[20];
        if (strcmp(metrics[i].name, "Non-Preemptive SJF") == 0) {
            strcpy(short_name, "NP SJF");
        } else if (strcmp(metrics[i].name, "Preemptive SJF") == 0) {
            strcpy(short_name, "P SJF");
        } else if (strcmp(metrics[i].name, "Non-Preemptive Priority") == 0) {
            strcpy(short_name, "NP Priority");
        } else if (strcmp(metrics[i].name, "Preemptive Priority") == 0) {
            strcpy(short_name, "P Priority");
        } else if (strcmp(metrics[i].name, "Priority with Aging") == 0) {
            strcpy(short_name, "Priority+Aging");
        } else {
            strcpy(short_name, metrics[i].name);
        }

        // EDF, RMS의 경우 turnaround time을 "-"로 표시
        if (i == 7 || i == 8) { // EDF, RMS
            printf("| %-20s | %13.2f | %16s | %11.2f%% | %10.4f | %16d |\n",
                   short_name, metrics[i].avg_wait_time, "-",
                   metrics[i].cpu_utilization, metrics[i].throughput,
                   metrics[i].missed_deadlines);
        } else { // 일반 알고리즘들
            printf("| %-20s | %13.2f | %16.2f | %11.2f%% | %10.4f | %16s |\n",
                   short_name, metrics[i].avg_wait_time,
                   metrics[i].avg_turnaround_time, metrics[i].cpu_utilization,
                   metrics[i].throughput, "N/A");
        }
    }

    printf(
        "+----------------------+---------------+------------------+----------"
        "----+------------+------------------+\n");

    // 리포트 생성
    FILE *fp = fopen("result_example/scheduling_comparison_report.txt", "w");
    if (fp) {
        fprintf(fp,
                "===== CPU Scheduling Simulator Comparison Report =====\n\n");
        fprintf(fp, "* Simulation Configuration:\n");
        fprintf(fp, " - Number of processes: %d\n", count);
        fprintf(fp, " - Time quantum (RR): %d\n", config->time_quantum);
        fprintf(fp, " - Priority range: 1-5 (1 being highest)\n");
        fprintf(fp, " - CPU burst time range: 2-10\n");
        fprintf(fp, " - I/O burst time range: 1-5 (when cpu burst is higher "
                    "than 2)\n\n");

        fprintf(fp, "* Performance Metrics Summary:\n");
        fprintf(fp,
                " +----------------------+---------------+------------------"
                "+--------------+------------+----------------+\n");
        fprintf(fp,
                " | Algorithm            | Avg Wait Time | Avg Turnaround   "
                "| CPU Util (%%) | Throughput | Missed Deadlines |\n");
        fprintf(fp,
                " +----------------------+---------------+------------------"
                "+--------------+------------+----------------+\n");

        for (int i = 0; i < 9; i++) {
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
            } else if (strcmp(metrics[i].name, "Priority with Aging") == 0) {
                strcpy(short_name, "Priority+Aging");
            } else {
                strcpy(short_name, metrics[i].name);
            }

            if (i == 7 || i == 8) { // EDF, RMS
                fprintf(
                    fp,
                    " | %-20s | %13.2f | %16s | %11.2f%% | %10.4f | %14d |\n",
                    short_name, metrics[i].avg_wait_time, "-",
                    metrics[i].cpu_utilization, metrics[i].throughput,
                    metrics[i].missed_deadlines);
            } else {
                fprintf(
                    fp,
                    " | %-20s | %13.2f | %16.2f | %11.2f%% | %10.4f | %14s |\n",
                    short_name, metrics[i].avg_wait_time,
                    metrics[i].avg_turnaround_time, metrics[i].cpu_utilization,
                    metrics[i].throughput, "N/A");
            }
        }

        fprintf(fp,
                " +----------------------+---------------+------------------"
                "+--------------+------------+----------------+\n\n");

        fprintf(fp, "* Algorithm Analysis:\n");

        // 평균 대기 시간이 가장 짧은 알고리즘 찾기 (일반 알고리즘만)
        int min_wait_idx = 0;
        for (int i = 1; i < 7; i++) { // EDF, RMS 제외
            if (metrics[i].avg_wait_time <
                metrics[min_wait_idx].avg_wait_time) {
                min_wait_idx = i;
            }
        }
        fprintf(fp, " - Minimum Average Waiting Time (General): %s (%.2f)\n",
                metrics[min_wait_idx].name,
                metrics[min_wait_idx].avg_wait_time);

        // 평균 반환 시간이 가장 짧은 알고리즘 찾기 (일반 알고리즘만)
        int min_turnaround_idx = 0;
        for (int i = 1; i < 7; i++) { // EDF, RMS 제외
            if (metrics[i].avg_turnaround_time <
                metrics[min_turnaround_idx].avg_turnaround_time) {
                min_turnaround_idx = i;
            }
        }
        fprintf(fp, " - Minimum Average Turnaround Time (General): %s (%.2f)\n",
                metrics[min_turnaround_idx].name,
                metrics[min_turnaround_idx].avg_turnaround_time);

        // CPU 사용률이 가장 높은 알고리즘 찾기
        int max_cpu_util_idx = 0;
        for (int i = 1; i < 9; i++) {
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
        for (int i = 1; i < 9; i++) {
            if (metrics[i].throughput >
                metrics[max_throughput_idx].throughput) {
                max_throughput_idx = i;
            }
        }
        fprintf(fp, " - Maximum Throughput: %s (%.4f)\n",
                metrics[max_throughput_idx].name,
                metrics[max_throughput_idx].throughput);

        // 실시간 스케줄링 성능 분석
        fprintf(fp, "\n* Real-Time Scheduling Analysis:\n");
        fprintf(fp, " - EDF Deadline Misses: %d\n",
                metrics[7].missed_deadlines);
        fprintf(fp, " - RMS Deadline Misses: %d\n",
                metrics[8].missed_deadlines);

        if (metrics[7].missed_deadlines == 0 &&
            metrics[8].missed_deadlines == 0) {
            fprintf(fp, " - Both EDF and RMS successfully met all deadlines\n");
        } else if (metrics[7].missed_deadlines < metrics[8].missed_deadlines) {
            fprintf(fp, " - EDF performed better with fewer deadline misses\n");
        } else if (metrics[8].missed_deadlines < metrics[7].missed_deadlines) {
            fprintf(fp, " - RMS performed better with fewer deadline misses\n");
        } else {
            fprintf(fp, " - EDF and RMS had equal deadline miss performance\n");
        }

        fprintf(fp, "\n* Conclusion:\n");
        fprintf(fp,
                " - For interactive systems: %s is ideal (lowest wait time: "
                "%.2f ms)\n",
                metrics[min_wait_idx].name,
                metrics[min_wait_idx].avg_wait_time);
        fprintf(fp,
                " - For batch processing: %s performs best (lowest turnaround: "
                "%.2f ms)\n",
                metrics[min_turnaround_idx].name,
                metrics[min_turnaround_idx].avg_turnaround_time);
        fprintf(fp,
                " - For maximum resource utilization: %s (%.2f%% CPU "
                "utilization)\n",
                metrics[max_cpu_util_idx].name,
                metrics[max_cpu_util_idx].cpu_utilization);

        fclose(fp);
        printf("Comparison report generated and saved as "
               "'scheduling_comparison_report.txt'\n");
    } else {
        printf("\nError: Could not create comparison report file.\n");
    }

    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }

    free(copy_processes);
    free(metrics_temp);
}

void display_performance_summary(Process *processes, int count, int total_time,
                                 int idle_time) {
    int total_waiting = 0;
    int total_turnaround = 0;
    int total_io_operations = 0;
    int total_io_time = 0;

    for (int i = 0; i < count; i++) {
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;

        // 멀티 I/O 통계 계산
        int process_io_count = get_io_count(&processes[i]);
        total_io_operations += process_io_count;

        for (int j = 0; j < MAX_IO_OPERATIONS; j++) {
            if (processes[i].io_operations[j].io_start != -1) {
                total_io_time += processes[i].io_operations[j].io_burst;
            }
        }
    }

    float avg_waiting = (float)total_waiting / count;
    float avg_turnaround = (float)total_turnaround / count;
    float cpu_utilization =
        ((float)(total_time - idle_time) / total_time) * 100.0;
    float avg_io_per_process = (float)total_io_operations / count;
    float io_intensity =
        (total_time > 0) ? ((float)total_io_time / total_time) * 100.0 : 0.0;

    printf("\n");
    print_section_divider("Performance Metrics with Multi-I/O Analysis", 62);

    // 각 줄을 정확히 70자로 맞춤 (양쪽 │ 포함하면 72자)
    printf(
        "│ Average Waiting Time       : %6.2f ms                           │\n",
        avg_waiting);
    printf(
        "│ Average Turnaround Time    : %6.2f ms                           │\n",
        avg_turnaround);
    printf("│ CPU Utilization            : %6.2f %%                            "
           "│\n",
           cpu_utilization);
    printf(
        "│ Total I/O Operations       : %6d operations                   │\n",
        total_io_operations);
    printf(
        "│ Average I/O per Process    : %6.2f operations                   │\n",
        avg_io_per_process);
    printf(
        "│ Total I/O Time             : %6d time units                   │\n",
        total_io_time);
    printf("│ I/O Intensity              : %6.2f %% of total time              "
           "│\n",
           io_intensity);
    printf("│ System Idle Time           : %6d time units (%5.2f %%)         "
           "│\n",
           idle_time, ((float)idle_time / total_time) * 100.0);

    print_section_end(66);
}

void display_process_table(Process *processes, int count) {
    printf("\n");
    printf("** Process Table with Multi-I/O Information **\n");

    // 기본 프로세스 정보 테이블
    printf("+-----+------------+-------------+----------+--------------+-------"
           "------+----------------+----------+----------+------------+\n");
    printf(
        "| PID | Burst Time | Arrive Time | Priority | Return Time  | Waiting "
        "Time| Turnaround Time| Deadline | Period   | Missed DL  |\n");
    printf("+-----+------------+-------------+----------+--------------+-------"
           "------+----------------+----------+----------+------------+\n");

    for (int i = 0; i < count; i++) {
        char missed_dl_str[10] = "No";
        if (processes[i].missed_deadline) {
            strcpy(missed_dl_str, "Yes");
        }

        printf("| P%-2d |     %-6d |      %-6d |    %-5d |       %-6d |      "
               "%-6d |       %-8d |   %-6d |   %-6d | %-10s |\n",
               processes[i].pid, processes[i].cpu_burst,
               processes[i].arrival_time, processes[i].priority,
               processes[i].comp_time, processes[i].waiting_time,
               processes[i].turnaround_time, processes[i].deadline,
               processes[i].period, missed_dl_str);
    }
    printf("+-----+------------+-------------+----------+--------------+-------"
           "------+----------------+----------+----------+------------+\n");

    // 멀티 I/O 상세 정보 테이블 - 수정된 버전
    printf("\n** Multi-I/O Operations Detail **\n");

    // 헤더 라인 1
    printf("+-----+----------+");
    for (int i = 1; i <= MAX_IO_OPERATIONS; i++) {
        printf("-------------+");
    }
    printf("--------------+\n");

    // 헤더 라인 2 - 컬럼 제목
    printf("| PID | I/O Count|");
    for (int i = 1; i <= MAX_IO_OPERATIONS; i++) {
        printf("   I/O %-2d    |", i);
    }
    printf("  I/O Total   |\n");

    // 헤더 라인 3 - 서브 헤더
    printf("|     |          |");
    for (int i = 1; i <= MAX_IO_OPERATIONS; i++) {
        printf(" Start:Burst |");
    }
    printf("              |\n");

    // 구분선
    printf("+-----+----------+");
    for (int i = 1; i <= MAX_IO_OPERATIONS; i++) {
        printf("-------------+");
    }
    printf("--------------+\n");

    // 데이터 행들
    for (int i = 0; i < count; i++) {
        int io_count = get_io_count(&processes[i]);
        int total_io_time = 0;

        printf("| P%-2d |    %-5d |", processes[i].pid, io_count);

        // 각 I/O 작업 정보 출력
        for (int j = 0; j < MAX_IO_OPERATIONS; j++) {
            if (processes[i].io_operations[j].io_start != -1) {
                printf("   %2d:%-4d   |",
                       processes[i].io_operations[j].io_start,
                       processes[i].io_operations[j].io_burst);
                total_io_time += processes[i].io_operations[j].io_burst;
            } else {
                printf("     --      |");
            }
        }

        printf("      %-6d  |\n", total_io_time);
    }

    // 마지막 구분선
    printf("+-----+----------+");
    for (int i = 1; i <= MAX_IO_OPERATIONS; i++) {
        printf("-------------+");
    }
    printf("--------------+\n");
}

// 추가: I/O 통계 요약 함수
void display_io_statistics(Process *processes, int count) {
    printf("\n** I/O Statistics Summary **\n");

    int processes_with_io = 0;
    int total_io_operations = 0;
    int min_io_burst = INT16_MAX;
    int max_io_burst = 0;
    int total_io_time = 0;

    for (int i = 0; i < count; i++) {
        int process_io_count = get_io_count(&processes[i]);
        if (process_io_count > 0) {
            processes_with_io++;
            total_io_operations += process_io_count;

            for (int j = 0; j < MAX_IO_OPERATIONS; j++) {
                if (processes[i].io_operations[j].io_start != -1) {
                    int burst = processes[i].io_operations[j].io_burst;
                    total_io_time += burst;
                    if (burst < min_io_burst)
                        min_io_burst = burst;
                    if (burst > max_io_burst)
                        max_io_burst = burst;
                }
            }
        }
    }

    printf("┌─────────────────────────────────────────────────────────────┐\n");
    printf("│ Processes with I/O        : %3d out of %3d                  │\n",
           processes_with_io, count);
    printf("│ Total I/O Operations      : %3d operations                  │\n",
           total_io_operations);
    printf("│ Average I/O per Process   : %.2f operations                 │\n",
           (count > 0) ? (float)total_io_operations / count : 0.0);
    printf("│ Total I/O Time            : %3d time units                  │\n",
           total_io_time);

    if (total_io_operations > 0) {
        printf(
            "│ Average I/O Duration      : %.2f time units                 │\n",
            (float)total_io_time / total_io_operations);
        printf(
            "│ Shortest I/O Burst       : %3d time units                   │\n",
            min_io_burst);
        printf(
            "│ Longest I/O Burst        : %3d time units                   │\n",
            max_io_burst);
    } else {
        printf("│ No I/O Operations Found                                     "
               "│\n");
    }

    printf("└─────────────────────────────────────────────────────────────┘\n");
}

GanttChart *consolidate_gantt_chart(GanttChart *original) {
    if (original->count == 0)
        return original;

    GanttChart *consolidated = malloc(sizeof(GanttChart));
    consolidated->entries = malloc(sizeof(GanttEntry) * original->count);
    consolidated->count = 0;
    consolidated->capacity = original->count;

    consolidated->entries[0] = original->entries[0];
    consolidated->count = 1;

    for (int i = 1; i < original->count; i++) {
        int last_idx = consolidated->count - 1; // 마지막 인덱스 말하는거

        if (consolidated->entries[last_idx].process_id ==
            original->entries[i].process_id) {
            consolidated->entries[last_idx].time_end =
                original->entries[i].time_end;
        } else {
            consolidated->entries[consolidated->count] = original->entries[i];
            consolidated->count++;
        }
    }

    return consolidated;
}

void display_gantt_chart(GanttChart *gantt, const char *algorithm_name) {
    printf("\n** Gantt Chart for %s **\n\n", algorithm_name);
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
            for (j = 0; j < duration - 1; j++) {
                printf(" "); // duration -1 인 이유는 이게 절반 이기 때문임 ->
                             // -- 이게 두문자니까
            }
            printf("ID");
            for (j = 0; j < duration - 1; j++) {
                printf(" ");
            }
        } else {
            char pid_str[10];
            sprintf(
                pid_str, "P%d",
                consolidated->entries[i]
                    .process_id); // 문자열 길이 계산 가능해짐 -> 숫자를 문자로

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

        if (end_time > 9) {
            printf("\b");
        }

        if (end_time > 99) {
            printf("\b");
        }

        printf("%d", end_time);
    }
    printf("\n");

    free(consolidated->entries);
    free(consolidated);
}

// evaluation.c 파일에 추가할 함수들

void save_processes_to_file(Process *processes, int count) {
    char filename[256];
    char filepath[300];
    char save_choice;

    printf("\n");
    printf("Do you want to save the current process configuration? (y/n): ");
    scanf(" %c", &save_choice);

    if (save_choice == 'y' || save_choice == 'Y') {
        printf("Enter filename (without extension): ");
        scanf("%s", filename);

        // 파일 경로 생성: test_files/process-filename.txt
        snprintf(filepath, sizeof(filepath), "test_files/process-%s.txt",
                 filename);

        FILE *file = fopen(filepath, "w");
        if (file == NULL) {
            printf("Error: Could not create file '%s'\n", filepath);
            printf("Make sure the 'test_files' directory exists.\n");
            return;
        }

        // 파일 형식에 맞게 저장
        // 첫 줄에 프로세스 개수
        fprintf(file, "%d\n", count);

        // 각 프로세스 정보 저장
        for (int i = 0; i < count; i++) {
            // 기본 프로세스 정보: PID 도착시간 CPU버스트 우선순위 데드라인 주기
            // IO개수
            int io_count = get_io_count(&processes[i]);
            fprintf(file, "%d %d %d %d %d %d %d\n", processes[i].pid,
                    processes[i].arrival_time, processes[i].cpu_burst,
                    processes[i].priority, processes[i].deadline,
                    processes[i].period, io_count);

            // 각 I/O 작업 정보 저장
            for (int j = 0; j < MAX_IO_OPERATIONS; j++) {
                if (processes[i].io_operations[j].io_start != -1) {
                    fprintf(file, "%d %d\n",
                            processes[i].io_operations[j].io_start,
                            processes[i].io_operations[j].io_burst);
                }
            }
        }

        fclose(file);
        printf("✓ Process configuration saved successfully as '%s'\n",
               filepath);
        printf("  You can load this configuration later by selecting file "
               "mode.\n");
    } else {
        printf("Process configuration not saved.\n");
    }
}

// 저장된 파일 목록을 보여주는 함수 (선택사항)
void list_saved_process_files() {
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;

    dir = opendir("test_files");
    if (dir == NULL) {
        printf("Warning: 'test_files' directory not found.\n");
        return;
    }

    printf("\n** Available Saved Process Files **\n");
    printf("-----------------------------------\n");

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "process-", 8) == 0) {
            char *ext = strstr(entry->d_name, ".txt");
            if (ext && strcmp(ext, ".txt") == 0) {
                // "process-" 접두사 제거하고 ".txt" 확장자 제거
                char name[256];
                strncpy(name, entry->d_name + 8, strlen(entry->d_name) - 12);
                name[strlen(entry->d_name) - 12] = '\0';

                printf("  - %s\n", name);
                file_count++;
            }
        }
    }

    if (file_count == 0) {
        printf("  No saved process files found.\n");
    } else {
        printf("-----------------------------------\n");
        printf("Total: %d saved configurations\n", file_count);
    }

    closedir(dir);
}

// 파일 저장 예시를 보여주는 함수
void show_file_format_example() {
    printf("\n** File Format Example **\n");
    printf("File: test_files/process-example.txt\n");
    printf("-----------------------------------\n");
    printf("4                    # Number of processes\n");
    printf("0 2 9 1 14 12 1      # P0: PID arrival CPU priority deadline "
           "period io_count\n");
    printf("3 4                  # I/O1: start_time burst_time\n");
    printf("1 5 9 9 20 15 2      # P1: 2 I/O operations\n");
    printf("3 5                  # I/O1: start_time burst_time\n");
    printf("6 2                  # I/O2: start_time burst_time\n");
    printf("2 8 6 10 20 12 0     # P2: No I/O operations\n");
    printf("3 3 4 7 12 9 1       # P3: 1 I/O operation\n");
    printf("2 3                  # I/O1: start_time burst_time\n");
    printf("-----------------------------------\n");
}

// main.c나 적절한 위치에서 호출할 수 있는 메뉴 함수
void show_file_management_menu() {
    char choice;

    printf("\n** File Management Options **\n");
    printf("1. List saved process files\n");
    printf("2. Show file format example\n");
    printf("3. Continue\n");
    printf("Select option (1-3): ");
    scanf(" %c", &choice);

    switch (choice) {
    case '1':
        list_saved_process_files();
        break;
    case '2':
        show_file_format_example();
        break;
    case '3':
    default:
        break;
    }
}

void display_scheduling_results(Process *processes, int count,
                                GanttChart *gantt, int total_time,
                                int idle_time, const char *algorithm_name) {
    display_gantt_chart(gantt, algorithm_name);

    display_process_table(processes, count);

    display_performance_summary(processes, count, total_time, idle_time);

    display_io_statistics(processes, count);
}
