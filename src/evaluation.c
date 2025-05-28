
#include "evaluation.h"
#include "process.h"
#include "queue.h"
#include "scheduler.h"
#include <dirent.h> // DIR, struct dirent, opendir, readdir, closedir
#include <limits.h> // INT_MAX 등의 상수 정의
#include <math.h>
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

void compare_algorithms(Process *processes, int count, Config *config,
                        int max_time) {
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

    // 9. RMS
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_rms(processes, count, config, max_time);
    int rms_counter = metrics_temp->for_edf_rms_counter;

    total_waiting = 0;
    int rms_completed_processes = 0;
    int rms_missed_deadlines = config->deadline_miss_info_count;

    for (int i = 0; i < rms_counter; i++) {
        if (metrics_temp->for_edf_rms_processes[i].comp_time > 0) {
            total_waiting +=
                metrics_temp->for_edf_rms_processes[i].waiting_time;
            rms_completed_processes++;
        }
    }

    strcpy(metrics[8].name, "RMS");
    metrics[8].avg_wait_time =
        rms_completed_processes > 0 ? (float)total_waiting / rms_counter : 0.0;
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

    // 8. EDF
    for (int i = 0; i < count; i++) {
        processes[i] = copy_processes[i];
    }
    reset_processes(processes, count);
    metrics_temp = run_edf(processes, count, config, max_time);
    int edf_counter = metrics_temp->for_edf_rms_counter;

    total_waiting = 0;
    int edf_completed_processes = 0;
    int edf_missed_deadlines = config->deadline_miss_info_count;

    for (int i = 0; i < edf_counter; i++) {
        if (metrics_temp->for_edf_rms_processes[i].comp_time > 0) {
            total_waiting +=
                metrics_temp->for_edf_rms_processes[i].waiting_time;
            edf_completed_processes++;
        }
    }

    strcpy(metrics[7].name, "EDF");
    metrics[7].avg_wait_time =
        edf_completed_processes > 0 ? (float)total_waiting / edf_counter : 0.0;
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

    // 사용자로부터 리포트 파일명 입력받기
    char report_filename[256];
    printf("\n  Enter report filename (without extension): ");
    scanf("%255s", report_filename);

    // 파일 경로 생성
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "result_example/%s.txt",
             report_filename);

    // 상세한 리포트 생성
    FILE *fp = fopen(full_path, "w");
    if (fp) {
        fprintf(fp, "=========================================================="
                    "=============================\n");
        fprintf(fp, "                    CPU SCHEDULING SIMULATOR - "
                    "COMPREHENSIVE REPORT\n");
        fprintf(fp, "=========================================================="
                    "=============================\n\n");

        // 시뮬레이션 설정 정보
        fprintf(fp, "SIMULATION CONFIGURATION:\n");
        fprintf(fp, "-------------------------\n");
        fprintf(fp, "• Number of processes: %d\n", count);
        fprintf(fp, "• Time quantum (Round Robin): %d time units\n",
                config->time_quantum);
        fprintf(fp, "• Simulation duration: Variable (until completion)\n");
        fprintf(fp, "• Multi-I/O support: Enabled\n");
        fprintf(fp, "• Priority range: 1-10 (1 = highest priority)\n");
        fprintf(fp, "• CPU burst range: 1-10 time units\n");
        fprintf(fp, "• I/O burst range: 1-5 time units\n\n");

        // 프로세스 상세 정보
        fprintf(fp, "PROCESS CONFIGURATION:\n");
        fprintf(fp, "----------------------\n");
        fprintf(fp, "+-----+--------+----------+----------+----------+---------"
                    "-+----------+\n");
        fprintf(fp, "| PID | Arrival| CPU Burst| Priority | Deadline | Period  "
                    " | I/O Ops  |\n");
        fprintf(fp, "+-----+--------+----------+----------+----------+---------"
                    "-+----------+\n");

        float total_theoretical_util = 0.0;
        for (int i = 0; i < count; i++) {
            // I/O 시간 계산
            int total_io_time = 0;
            int io_count = 0;
            for (int j = 0; j < MAX_IO_OPERATIONS; j++) {
                if (processes[i].io_operations[j].io_start != -1) {
                    total_io_time += processes[i].io_operations[j].io_burst;
                    io_count++;
                }
            }

            int execution_time = processes[i].cpu_burst + total_io_time;
            float individual_util = (float)execution_time / processes[i].period;
            total_theoretical_util += individual_util;

            fprintf(fp,
                    "| P%-2d |   %-4d |    %-5d |    %-5d |    %-5d |    %-5d "
                    "|    %-5d |\n",
                    i, processes[i].arrival_time, processes[i].cpu_burst,
                    processes[i].priority, processes[i].deadline,
                    processes[i].period, io_count);
        }
        fprintf(fp, "+-----+--------+----------+----------+----------+---------"
                    "-+----------+\n\n");

        // 이론적 Utilization 분석
        fprintf(fp, "UTILIZATION ANALYSIS:\n");
        fprintf(fp, "---------------------\n");
        fprintf(fp, "• Total System Utilization: %.3f (%.2f%%)\n",
                total_theoretical_util, total_theoretical_util * 100);

        float rms_bound = count * (pow(2.0, 1.0 / count) - 1);
        fprintf(fp, "• RMS Theoretical Bound: %.3f (%.2f%%)\n", rms_bound,
                rms_bound * 100);
        fprintf(fp, "• EDF Theoretical Bound: 1.000 (100.00%%)\n\n");

        fprintf(fp, "Schedulability Prediction:\n");
        fprintf(fp, "• EDF: %s (Utilization %s 100%%)\n",
                total_theoretical_util <= 1.0 ? "SCHEDULABLE"
                                              : "NOT SCHEDULABLE",
                total_theoretical_util <= 1.0 ? "<=" : ">");
        fprintf(
            fp, "• RMS: %s (Utilization %s %.2f%%)\n\n",
            total_theoretical_util <= rms_bound ? "SCHEDULABLE" : "UNCERTAIN",
            total_theoretical_util <= rms_bound ? "<=" : ">", rms_bound * 100);

        // 성능 메트릭 상세 표
        fprintf(fp, "PERFORMANCE METRICS COMPARISON:\n");
        fprintf(fp, "================================\n");
        fprintf(fp, "+----------------------+----------+----------+----------+-"
                    "---------+----------+------------+\n");
        fprintf(fp, "| Algorithm            | Avg Wait | Avg Turn | CPU Util | "
                    "Thru-put | DL Miss  | Efficiency |\n");
        fprintf(fp, "|                      | Time(ms) | Time(ms) |   (%%)    "
                    "| (proc/s) | Count    | Score      |\n");
        fprintf(fp, "+----------------------+----------+----------+----------+-"
                    "---------+----------+------------+\n");

        // 효율성 점수 계산을 위한 정규화
        float max_wait = 0, max_turn = 0, max_cpu = 0, max_throughput = 0;
        for (int i = 0; i < 9; i++) {
            if (i < 7) { // 일반 알고리즘만
                if (metrics[i].avg_wait_time > max_wait)
                    max_wait = metrics[i].avg_wait_time;
                if (metrics[i].avg_turnaround_time > max_turn)
                    max_turn = metrics[i].avg_turnaround_time;
            }
            if (metrics[i].cpu_utilization > max_cpu)
                max_cpu = metrics[i].cpu_utilization;
            if (metrics[i].throughput > max_throughput)
                max_throughput = metrics[i].throughput;
        }

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

            // 효율성 점수 계산 (낮은 대기시간, 높은 CPU 사용률이 좋음)
            float efficiency_score = 0.0;
            if (i < 7) { // 일반 알고리즘
                efficiency_score =
                    (1.0 - metrics[i].avg_wait_time / max_wait) * 0.4 +
                    (1.0 - metrics[i].avg_turnaround_time / max_turn) * 0.3 +
                    (metrics[i].cpu_utilization / max_cpu) * 0.3;
            } else { // 실시간 알고리즘
                efficiency_score =
                    (metrics[i].cpu_utilization / max_cpu) * 0.5 +
                    (metrics[i].missed_deadlines == 0 ? 1.0 : 0.0) * 0.5;
            }

            if (i == 7 || i == 8) { // EDF, RMS
                fprintf(
                    fp,
                    "| %-20s | %8s | %8s | %8.2f | %8.4f | %8d | %10.3f |\n",
                    short_name, "N/A", "N/A", metrics[i].cpu_utilization,
                    metrics[i].throughput, metrics[i].missed_deadlines,
                    efficiency_score);
            } else {
                fprintf(fp,
                        "| %-20s | %8.2f | %8.2f | %8.2f | %8.4f | %8s | "
                        "%10.3f |\n",
                        short_name, metrics[i].avg_wait_time,
                        metrics[i].avg_turnaround_time,
                        metrics[i].cpu_utilization, metrics[i].throughput,
                        "N/A", efficiency_score);
            }
        }
        fprintf(fp, "+----------------------+----------+----------+----------+-"
                    "---------+----------+------------+\n\n");

        // 상세 분석
        fprintf(fp, "DETAILED ANALYSIS:\n");
        fprintf(fp, "==================\n\n");

        // 1. 일반 스케줄링 알고리즘 분석
        fprintf(fp, "1. GENERAL SCHEDULING ALGORITHMS:\n");
        fprintf(fp, "   -------------------------------\n");

        int min_wait_idx = 0, min_turn_idx = 0, max_cpu_gen_idx = 0;
        for (int i = 1; i < 7; i++) {
            if (metrics[i].avg_wait_time < metrics[min_wait_idx].avg_wait_time)
                min_wait_idx = i;
            if (metrics[i].avg_turnaround_time <
                metrics[min_turn_idx].avg_turnaround_time)
                min_turn_idx = i;
            if (metrics[i].cpu_utilization >
                metrics[max_cpu_gen_idx].cpu_utilization)
                max_cpu_gen_idx = i;
        }

        fprintf(fp, "   • Best for Interactive Systems: %s\n",
                metrics[min_wait_idx].name);
        fprintf(fp, "     - Lowest average waiting time: %.2f ms\n",
                metrics[min_wait_idx].avg_wait_time);
        fprintf(fp, "     - CPU utilization: %.2f%%\n\n",
                metrics[min_wait_idx].cpu_utilization);

        fprintf(fp, "   • Best for Batch Processing: %s\n",
                metrics[min_turn_idx].name);
        fprintf(fp, "     - Lowest average turnaround time: %.2f ms\n",
                metrics[min_turn_idx].avg_turnaround_time);
        fprintf(fp, "     - Throughput: %.4f processes/second\n\n",
                metrics[min_turn_idx].throughput);

        fprintf(fp, "   • Most Resource Efficient: %s\n",
                metrics[max_cpu_gen_idx].name);
        fprintf(fp, "     - Highest CPU utilization: %.2f%%\n",
                metrics[max_cpu_gen_idx].cpu_utilization);
        fprintf(fp, "     - System idle time minimized\n\n");

        // 2. 실시간 스케줄링 분석
        fprintf(fp, "2. REAL-TIME SCHEDULING ALGORITHMS:\n");
        fprintf(fp, "   ---------------------------------\n");
        fprintf(fp, "   • EDF (Earliest Deadline First):\n");
        fprintf(fp, "     - Deadline misses: %d\n",
                metrics[7].missed_deadlines);
        fprintf(fp, "     - CPU utilization: %.2f%%\n",
                metrics[7].cpu_utilization);
        fprintf(
            fp,
            "     - Theoretical optimality: Optimal up to 100%% utilization\n");
        fprintf(fp,
                "     - Dynamic priority assignment based on deadlines\n\n");

        fprintf(fp, "   • RMS (Rate Monotonic Scheduling):\n");
        fprintf(fp, "     - Deadline misses: %d\n",
                metrics[8].missed_deadlines);
        fprintf(fp, "     - CPU utilization: %.2f%%\n",
                metrics[8].cpu_utilization);
        fprintf(fp, "     - Theoretical bound: %.2f%% for %d processes\n",
                rms_bound * 100, count);
        fprintf(fp, "     - Fixed priority assignment based on periods\n\n");

        // 3. 실시간 성능 비교
        fprintf(fp, "3. REAL-TIME PERFORMANCE COMPARISON:\n");
        fprintf(fp, "   ----------------------------------\n");
        if (metrics[7].missed_deadlines == 0 &&
            metrics[8].missed_deadlines == 0) {
            fprintf(fp,
                    "   • Both algorithms successfully met all deadlines\n");
            fprintf(fp,
                    "   • System utilization (%.2f%%) is within both "
                    "algorithms' capabilities\n",
                    total_theoretical_util * 100);
        } else if (metrics[7].missed_deadlines < metrics[8].missed_deadlines) {
            fprintf(fp,
                    "   • EDF outperformed RMS with fewer deadline misses\n");
            fprintf(
                fp,
                "   • System utilization (%.2f%%) exceeds RMS bound (%.2f%%)\n",
                total_theoretical_util * 100, rms_bound * 100);
            fprintf(fp, "   • EDF's dynamic priority proved more effective\n");
        } else if (metrics[8].missed_deadlines < metrics[7].missed_deadlines) {
            fprintf(fp, "   • RMS outperformed EDF (unexpected result)\n");
            fprintf(fp, "   • This may indicate specific task timing patterns "
                        "favoring RMS\n");
        } else if (metrics[7].missed_deadlines > 0) {
            fprintf(fp,
                    "   • Both algorithms had equal deadline miss counts\n");
            fprintf(fp, "   • System utilization exceeds schedulable limits\n");
        }
        fprintf(fp, "\n");

        // 4. I/O 영향 분석
        fprintf(fp, "4. MULTI-I/O IMPACT ANALYSIS:\n");
        fprintf(fp, "   ---------------------------\n");
        int total_io_ops = 0;
        int total_io_time = 0;
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < MAX_IO_OPERATIONS; j++) {
                if (processes[i].io_operations[j].io_start != -1) {
                    total_io_ops++;
                    total_io_time += processes[i].io_operations[j].io_burst;
                }
            }
        }

        fprintf(fp, "   • Total I/O operations: %d\n", total_io_ops);
        fprintf(fp, "   • Total I/O time: %d time units\n", total_io_time);
        fprintf(fp, "   • Average I/O per process: %.2f operations\n",
                (float)total_io_ops / count);
        fprintf(fp, "   • I/O impact on scheduling: %s\n",
                total_io_ops > 0 ? "Significant - increases context switching"
                                 : "Minimal");
        fprintf(fp, "\n");

        // 5. 권장사항
        fprintf(fp, "5. RECOMMENDATIONS:\n");
        fprintf(fp, "   ----------------\n");
        fprintf(fp, "   Based on the analysis results:\n\n");

        fprintf(fp, "   • For General Purpose Systems:\n");
        fprintf(fp, "     - Use %s for lowest response time\n",
                metrics[min_wait_idx].name);
        fprintf(fp, "     - Use %s for highest throughput\n",
                metrics[min_turn_idx].name);
        fprintf(fp, "\n");

        fprintf(fp, "   • For Real-Time Systems:\n");
        if (total_theoretical_util <= rms_bound) {
            fprintf(fp, "     - Both EDF and RMS are suitable\n");
            fprintf(fp, "     - RMS preferred for predictable behavior\n");
            fprintf(fp, "     - EDF preferred for maximum utilization\n");
        } else if (total_theoretical_util <= 1.0) {
            fprintf(fp, "     - EDF is strongly recommended\n");
            fprintf(fp, "     - RMS may experience deadline misses\n");
            fprintf(fp, "     - Consider reducing system load for RMS\n");
        } else {
            fprintf(fp,
                    "     - System is overloaded for real-time guarantees\n");
            fprintf(fp, "     - Reduce task load or increase periods\n");
            fprintf(fp, "     - Consider task partitioning\n");
        }
        fprintf(fp, "\n");

        // 6. 시스템 특성
        fprintf(fp, "6. SYSTEM CHARACTERISTICS:\n");
        fprintf(fp, "   -----------------------\n");
        fprintf(fp, "   • Workload type: %s\n",
                total_io_ops > count ? "I/O Intensive" : "CPU Intensive");
        fprintf(fp, "   • Real-time feasibility: %s\n",
                total_theoretical_util <= 1.0 ? "Feasible with EDF"
                                              : "Overloaded");
        fprintf(fp, "   • Priority inversion risk: %s\n",
                total_io_ops > 0 ? "Present due to I/O operations" : "Low");
        fprintf(fp, "   • Scalability: %s\n",
                count <= 4 ? "Good"
                           : "Consider partitioning for large task sets");

        fprintf(fp, "\n");
        fprintf(fp, "=========================================================="
                    "=====================\n");
        fprintf(fp, "Report generated by CPU Scheduling Simulator v1.0\n");
        fprintf(fp,
                "Analysis includes %d algorithms with comprehensive metrics\n",
                9);
        fprintf(fp, "Copyright © 2025 Seongmin Lee\n");
        fprintf(fp, "Licensed under the MIT License\n");
        fprintf(fp, "=========================================================="
                    "=====================\n");

        fclose(fp);
        printf("\nComprehensive scheduling analysis report generated!\n");
        printf("Saved as: 'result_example/%s.txt'\n", report_filename);
        printf("Report includes utilization analysis, performance metrics, "
               "and recommendations.\n\n");
    } else {
        printf("\nError: Could not create report file '%s.txt'\n",
               report_filename);
        printf("Please check if the 'result_example' directory exists.\n");
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
    printf("│ Processes with I/O        : %3d out of %-3d                  │\n",
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
            // IO 개수
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

void print_utilization_analysis(Process *original_processes, int count,
                                const char *algorithm_name) {
    printf("\n** Utilization Analysis for %s **\n", algorithm_name);
    printf("+------+-------------+----------+-------------+---------------+\n");
    printf(
        "| PID  | CPU + I/O   | Period   | Individual  | Utilization %% |\n");
    printf("+------+-------------+----------+-------------+---------------+\n");

    float total_utilization = 0.0;

    for (int i = 0; i < count; i++) {
        // I/O 시간 계산
        int total_io_time = 0;
        for (int j = 0; j < MAX_IO_OPERATIONS; j++) {
            if (original_processes[i].io_operations[j].io_start != -1) {
                total_io_time +=
                    original_processes[i].io_operations[j].io_burst;
            }
        }

        int execution_time = original_processes[i].cpu_burst + total_io_time;
        float individual_util =
            (float)execution_time / original_processes[i].period;
        total_utilization += individual_util;

        printf("| P%-3d | %-11d | %-8d | %-11.3f | %-13.2f |\n", i,
               execution_time, original_processes[i].period, individual_util,
               individual_util * 100);
    }

    printf("+------+-------------+----------+-------------+---------------+\n");
    printf("| Total System Utilization: %.3f (%.2f%%)                    |\n",
           total_utilization, total_utilization * 100);

    float rms_bound = count * (pow(2.0, 1.0 / count) - 1);

    if (strcmp(algorithm_name, "EDF") == 0) {
        printf(
            "| %s Theoretical Bound: %.3f (%.2f%%)                       |\n",
            algorithm_name, 1.0, 100.0);
    } else if (strcmp(algorithm_name, "RMS") == 0) {
        printf(
            "| %s Theoretical Bound: %.3f (%.2f%%)                       |\n",
            algorithm_name, rms_bound, rms_bound * 100);
    }

    printf("+------+-------------+----------+-------------+---------------+\n");

    // 스케줄링 가능성 예측
    printf("\n** Schedulability Prediction **\n");
    printf("- EDF: %s (Utilization <= 100%%)\n",
           total_utilization <= 1.0 ? "SCHEDULABLE" : "NOT SCHEDULABLE");
    printf("- RMS: %s (Utilization <= %.2f%%)\n",
           total_utilization <= rms_bound ? "SCHEDULABLE" : "UNCERTAIN",
           rms_bound * 100);
    printf("\n");
}

void display_scheduling_results(Process *processes, int count,
                                GanttChart *gantt, int total_time,
                                int idle_time, const char *algorithm_name) {
    display_gantt_chart(gantt, algorithm_name);

    display_process_table(processes, count);

    display_performance_summary(processes, count, total_time, idle_time);

    display_io_statistics(processes, count);
}
