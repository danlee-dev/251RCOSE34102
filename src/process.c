#include "process.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FILES 100

Process *create_processes(int *count, char mode) {
    Process *processes = (Process *)malloc(sizeof(Process) * *count);

    if (mode == 'y') {
        for (int i = 0; i < *count; i++) {
            processes[i].pid = i;
            processes[i].arrival_time = rand() % 10;
            processes[i].cpu_burst = rand() % 10 + 1;

            if (processes[i].cpu_burst > 2) {
                processes[i].io_start =
                    rand() % (processes[i].cpu_burst - 1) + 1;
                processes[i].io_burst = rand() % 5 + 1;
            } else {
                processes[i].io_start = -1;
                processes[i].io_burst = 0;
            }

            processes[i].priority = rand() % 10 + 1;
            processes[i].period = processes[i].cpu_burst + (rand() % 4) + 3;
            processes[i].deadline =
                processes[i].arrival_time + processes[i].period;

            processes[i].missed_deadline = 0;
            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
        }
    } else if (mode == 'n') {
        for (int i = 0; i < *count; i++) {
            printf("Enter process %d information:\n", i);

            printf("  PID: ");
            scanf("%d", &processes[i].pid);

            printf("  Arrival Time: ");
            scanf("%d", &processes[i].arrival_time);

            printf("  CPU Burst: ");
            scanf("%d", &processes[i].cpu_burst);

            printf("  IO Start: ");
            scanf("%d", &processes[i].io_start);

            printf("  IO Burst: ");
            scanf("%d", &processes[i].io_burst);

            printf("  Priority: ");
            scanf("%d", &processes[i].priority);

            printf("  Deadline: ");
            scanf("%d", &processes[i].deadline);

            printf("  Period: ");
            scanf("%d", &processes[i].period);

            processes[i].missed_deadline = 0;
            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
        }
    } else {
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
            printf("No valid process files found.\n");
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
            perror("fopen");
            return NULL;
        }

        int real_count = 0;
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            if (line[0] != '\n')
                real_count++;
        }

        rewind(file);

        processes = (Process *)malloc(sizeof(Process) * real_count);
        if (!processes) {
            perror("malloc");
            fclose(file);
            return NULL;
        }

        *count = real_count;

        for (int i = 0; i < *count; i++) {
            if (fscanf(file, "%d %d %d %d %d %d %d %d", &processes[i].pid,
                       &processes[i].arrival_time, &processes[i].cpu_burst,
                       &processes[i].io_start, &processes[i].io_burst,
                       &processes[i].priority, &processes[i].deadline,
                       &processes[i].period) != 8) {
                printf("Failed to read process %d from file.\n", i);
                fclose(file);
                return NULL;
            }

            processes[i].missed_deadline = 0;
            processes[i].remaining_time = processes[i].cpu_burst;
            processes[i].progress = 0;
            processes[i].comp_time = 0;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].waiting_time_counter = 0;
        }

        fclose(file);

        for (int i = 0; i < count_files; i++) {
            free(filenames[i]);
        }
    }

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
