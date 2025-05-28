#include "process.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FILES 100

int rand_except(int min, int max, int exclude) {
    int r;
    if (max - min <= 0)
        return min; // 예외 처리
    do {
        r = rand() % (max - min + 1) + min;
    } while (r == exclude);
    return r;
}

// 프로세스의 I/O 작업 초기화
void init_process_io(Process *p) {
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        p->io_operations[i].io_start = -1;
        p->io_operations[i].io_burst = 0;
    }
}

// I/O 작업 추가 함수
void add_io_to_process(Process *p, int io_start, int io_burst) {
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start == -1) {
            p->io_operations[i].io_start = io_start;
            p->io_operations[i].io_burst = io_burst;
            break;
        }
    }
}

// I/O 작업 개수 반환
int get_io_count(Process *p) {
    int count = 0;
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start != -1) {
            count++;
        }
    }
    return count;
}

void sort_io_operations(Process *p) {
    // 버블 정렬을 사용하여 io_start 기준으로 오름차순 정렬
    for (int i = 0; i < MAX_IO_OPERATIONS - 1; i++) {
        for (int j = 0; j < MAX_IO_OPERATIONS - i - 1; j++) {
            // 둘 다 유효한 I/O 작업인 경우에만 비교
            if (p->io_operations[j].io_start != -1 &&
                p->io_operations[j + 1].io_start != -1) {

                // io_start 기준으로 정렬 (작은 값이 앞에 오도록)
                if (p->io_operations[j].io_start >
                    p->io_operations[j + 1].io_start) {
                    // 두 I/O 작업 교환
                    IOOperation temp = p->io_operations[j];
                    p->io_operations[j] = p->io_operations[j + 1];
                    p->io_operations[j + 1] = temp;
                }
            }
        }
    }
}

Process *create_processes(int *count, char mode) {
    Process *processes = (Process *)malloc(sizeof(Process) * *count);

    if (mode == 'y') {
        // 랜덤 모드
        printf("\nCreating processes with random Multi-I/O...\n");

        for (int i = 0; i < *count; i++) {
            processes[i].pid = i;
            processes[i].arrival_time = rand() % 10;
            processes[i].cpu_burst = rand() % 10 + 1;

            // I/O 배열 초기화
            init_process_io(&processes[i]);

            // 랜덤으로 0-3개의 I/O 작업 생성
            int io_count = rand() % 4; // 0, 1, 2, 3개 중 랜덤
            printf(" - Process P%d: CPU %d, I/O count: %d\n", i,
                   processes[i].cpu_burst, io_count);

            // I/O 시작 시간들을 저장할 배열 (중복 방지용)
            int used_start_times[MAX_IO_OPERATIONS];
            int used_count = 0;

            for (int j = 0; j < io_count; j++) {
                int start_time, duration;

                // CPU 버스트 시간이 2 이상이고, 사용 가능한 시간이 남아있을
                // 때만 I/O 생성
                if (processes[i].cpu_burst > 1 &&
                    used_count < processes[i].cpu_burst - 1) {
                    // 중복되지 않는 시작 시간 생성
                    int attempts = 0;
                    do {
                        start_time = rand() % (processes[i].cpu_burst - 1) +
                                     1; // start time을 최소 한번은 실행
                        attempts++;
                        // 무한루프 방지: 100번 시도해도 안되면 중단
                        if (attempts > 100) {
                            printf("   Warning: Cannot find more unique I/O "
                                   "start times for P%d\n",
                                   i);
                            break;
                        }
                    } while (
                        is_time_used(used_start_times, used_count, start_time));

                    // 성공적으로 시작 시간을 찾은 경우에만 I/O 추가
                    if (attempts <= 100) {
                        used_start_times[used_count++] = start_time;
                        duration = rand() % 5 + 1;

                        add_io_to_process(&processes[i], start_time, duration);

                        printf("    I/O %d: Start at %d, Duration %d\n", j + 1,
                               start_time, duration);
                    } else {
                        break; // 더 이상 I/O 생성 불가
                    }
                } else {
                    break; // CPU 버스트가 너무 짧거나 더 이상 공간이 없음
                }
            }

            sort_io_operations(&processes[i]);

            int total_io_time = 0;
            for (int j = 0; j < io_count; j++) {
                if (processes[i].io_operations[j].io_start > 0) {
                    total_io_time += processes[i].io_operations[j].io_burst;
                }
            }

            // 실제 실행시간 계산
            int actual_execution_time = processes[i].cpu_burst + total_io_time;

            // 프로세스 개수에 따른 목표 개별 utilization 계산
            float target_individual_utilization;
            if (*count <= 2) {
                target_individual_utilization = 0.4; // 40%
            } else if (*count <= 4) {
                target_individual_utilization = 0.25; // 25%
            } else if (*count <= 6) {
                target_individual_utilization = 0.18; // 18%
            } else {
                target_individual_utilization = 0.15; // 15%
            }

            // 목표 utilization을 약간 랜덤하게 변동
            float variation = 1.0 + (rand() % 40 - 20) / 100.0; // 0.8 ~ 1.2
            float adjusted_utilization =
                target_individual_utilization * variation;

            // Period 계산: actual_time / target_utilization
            int base_period =
                (int)(actual_execution_time / adjusted_utilization);
            processes[i].period = base_period + (rand() % 5) + 2;

            processes[i].deadline =
                processes[i].arrival_time + processes[i].period;

            printf(
                "   Process P%d: Execution=%d, Period=%d, Utilization=%.2f%%\n",
                i, actual_execution_time, processes[i].period,
                (float)actual_execution_time / processes[i].period * 100);

            processes[i].priority = rand() % 10 + 1;

            processes[i].missed_deadline = 0;
            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
        }

    } else if (mode == 'n') {
        // 직접 입력 모드
        printf("\nManual input mode with Multi-I/O support\n");

        for (int i = 0; i < *count; i++) {
            printf("\n=== Enter process %d information ===\n", i);

            printf("  PID: ");
            scanf("%d", &processes[i].pid);

            printf("  Arrival Time: ");
            scanf("%d", &processes[i].arrival_time);

            printf("  CPU Burst: ");
            scanf("%d", &processes[i].cpu_burst);

            printf("  Priority: ");
            scanf("%d", &processes[i].priority);

            printf("  Deadline: ");
            scanf("%d", &processes[i].deadline);

            printf("  Period: ");
            scanf("%d", &processes[i].period);

            // I/O 배열 초기화
            init_process_io(&processes[i]);

            // I/O 작업 개수 입력
            int io_count;
            printf("  Number of I/O operations (0-%d): ", MAX_IO_OPERATIONS);
            scanf("%d", &io_count);

            if (io_count < 0)
                io_count = 0;
            if (io_count > MAX_IO_OPERATIONS)
                io_count = MAX_IO_OPERATIONS;

            // 각 I/O 작업 정보 입력
            for (int j = 0; j < io_count; j++) {
                int io_start, io_burst;

                printf("  I/O %d:\n", j + 1);
                printf("    Start time (1-%d): ", processes[i].cpu_burst - 1);
                scanf("%d", &io_start);

                printf("    Burst time: ");
                scanf("%d", &io_burst);

                add_io_to_process(&processes[i], io_start, io_burst);
            }

            processes[i].missed_deadline = 0;
            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
            sort_io_operations(&processes[i]);
        }

    } else {
        // 파일에서 읽기 모드
        DIR *dir;
        struct dirent *entry;
        char *filenames[MAX_FILES];
        int count_files = 0;

        dir = opendir("test_files");
        if (dir == NULL) {
            perror("opendir");
            return NULL;
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, "process-", 8) == 0) {
                char *ext = strstr(entry->d_name, ".txt");
                if (ext && strcmp(ext, ".txt") == 0) {
                    filenames[count_files] = strdup(entry->d_name);
                    count_files++;
                    if (count_files >= MAX_FILES)
                        break;
                }
            }
        }
        closedir(dir);

        if (count_files == 0) {
            printf("No valid process files in 'test_files/' directory.\n");
            return NULL;
        }

        printf("Available process files:\n");
        for (int i = 0; i < count_files; i++) {
            printf("[%d] %s\n", i + 1, filenames[i]);
        }

        int choice;
        printf("Select a file to load (1 ~ %d): ", count_files);
        scanf("%d", &choice);

        if (choice < 1 || choice > count_files) {
            printf("Invalid selection.\n");
            return NULL;
        }

        char filepath[256];
        snprintf(filepath, sizeof(filepath), "test_files/%s",
                 filenames[choice - 1]);
        FILE *file = fopen(filepath, "r");

        if (!file) {
            printf("Cannot open file: %s\n", filepath);
            return NULL;
        }

        // 프로세스 개수 먼저 읽기
        int real_count = 0;
        if (fscanf(file, "%d", &real_count) != 1) {
            printf("Failed to read process count from file.\n");
            fclose(file);
            return NULL;
        }

        processes = (Process *)malloc(sizeof(Process) * real_count);
        if (!processes) {
            perror("malloc error");
            fclose(file);
            return NULL;
        }

        *count = real_count;

        // 각 프로세스 정보 읽기
        for (int i = 0; i < *count; i++) {
            int io_count;

            // 기본 프로세스 정보 읽기
            if (fscanf(file, "%d %d %d %d %d %d %d", &processes[i].pid,
                       &processes[i].arrival_time, &processes[i].cpu_burst,
                       &processes[i].priority, &processes[i].deadline,
                       &processes[i].period, &io_count) != 7) {
                printf("Failed to read process %d basic info from file.\n", i);
                fclose(file);
                return NULL;
            }

            // I/O 배열 초기화
            init_process_io(&processes[i]);

            // I/O 작업들 읽기
            for (int j = 0; j < io_count && j < MAX_IO_OPERATIONS; j++) {
                int io_start, io_burst;

                if (fscanf(file, "%d %d", &io_start, &io_burst) != 2) {
                    printf("Failed to read I/O %d info for process %d from "
                           "file.\n",
                           j, i);
                    fclose(file);
                    return NULL;
                }

                add_io_to_process(&processes[i], io_start, io_burst);
            }

            processes[i].missed_deadline = 0;
            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
            sort_io_operations(&processes[i]);
        }

        fclose(file);

        for (int i = 0; i < count_files; i++) {
            free(filenames[i]);
        }
    }

    // PID 기준으로 정렬
    for (int i = 0; i < *count - 1; i++) {
        for (int j = 0; j < *count - i - 1; j++) {
            if (processes[j].pid > processes[j + 1].pid) {
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }

    return processes;
}

// 시간이 이미 사용되었는지 확인하는 함수
int is_time_used(int *used_times, int count, int time) {
    for (int i = 0; i < count; i++) {
        if (used_times[i] == time) {
            return 1;
        }
    }
    return 0;
}

// 특정 진행도에서 I/O가 시작되는지 확인
int has_io_at_progress(Process *p, int progress) {
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start == progress) {
            return 1;
        }
    }
    return 0;
}

// 특정 진행도에서의 I/O 버스트 시간 반환
int get_io_burst_at_progress(Process *p, int progress) {
    for (int i = 0; i < MAX_IO_OPERATIONS; i++) {
        if (p->io_operations[i].io_start == progress) {
            return p->io_operations[i].io_burst;
        }
    }
    return 0;
}

void reset_processes(Process *processes, int count) {
    for (int i = 0; i < count; i++) {
        processes[i].remaining_time = processes[i].cpu_burst;
        processes[i].progress = 0;
        processes[i].comp_time = 0;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time_counter = 0;
        processes[i].missed_deadline = 0;
    }
}
