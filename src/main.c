#include "config.h"
#include "evaluation.h"
#include "process.h"
#include "queue.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));

    int max_time; // for EDF, RMS

    int choice;
    int *process_count = malloc(sizeof(int));

    printf("===== CPU Scheduling Simulator =====\n");

    printf("random create mode? ('y': yes, 'n': no, 'f': Use file): ");
    char mode;
    scanf(" %c", &mode);

    if (mode != 'f') {
        printf("\nEnter the number of processes: ");
        scanf("%d", process_count);
    }

    Process *processes = create_processes(process_count, mode);

    Config system_config;
    init_config(&system_config, mode);
    int count = *process_count;

    while (1) {
        printf("\n===== CPU Scheduling Algorithms =====\n");
        printf("1. Run FCFS\n");
        printf("2. Run SJF (Non-preemptive)\n");
        printf("3. Run SJF (Preemptive)\n");
        printf("4. Run Priority (Non-preemptive)\n");
        printf("5. Run Priority (Preemptive)\n");
        printf("6. Run Round Robin\n");
        printf("7. Run Priority with Aging (Preemptive)\n");
        printf("8. Run RMS (Rate Monotonic Scheduling)\n");
        printf("9. Run EDF (Earliest Deadline First)\n");
        printf("10. Compare all algorithms\n");
        printf("11. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            run_fcfs(processes, count);
            break;
        case 2:
            run_sjf_np(processes, count);
            break;
        case 3:
            run_sjf_p(processes, count);
            break;
        case 4:
            run_priority_np(processes, count);
            break;
        case 5:
            run_priority_p(processes, count);
            break;
        case 6:
            run_rr(processes, count, &system_config);
            break;
        case 7:
            run_priority_with_aging(processes, count);
            break;
        case 8:
            printf("Enter max time(for RMS): ");
            scanf("%d", &max_time);
            run_rms(processes, count, &system_config, max_time);
            break;
        case 9:
            printf("Enter max time(for EDF): ");
            scanf("%d", &max_time);
            run_edf(processes, count, &system_config, max_time);
            break;
        case 10:
            printf("Enter max time(for EDF, RMS): ");
            scanf("%d", &max_time);
            compare_algorithms(processes, count, &system_config, max_time);
            break;
        case 11:
            if (mode != 'f')
                save_processes_to_file(processes, count);
            free(processes);
            return 0;
        default:
            printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
