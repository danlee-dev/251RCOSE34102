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

    int choice;
    int process_count;

    printf("===== CPU Scheduling Simulator =====\n");
    printf("Enter the number of processes: ");
    scanf("%d", &process_count);

    // 프로세스 생성
    Process *processes = create_processes(process_count);

    // 시스템 설정
    Config system_config;
    init_config(&system_config);

    while (1) {
        printf("\n1. Run FCFS\n");
        printf("2. Run SJF (Non-preemptive)\n");
        printf("3. Run SJF (Preemptive)\n");
        printf("4. Run Priority (Non-preemptive)\n");
        printf("5. Run Priority (Preemptive)\n");
        printf("6. Run Round Robin\n");
        printf("7. Compare all algorithms\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            run_fcfs(processes, process_count, &system_config);
            break;
        case 2:
            run_sjf_np(processes, process_count, &system_config);
            break;
        case 3:
            run_sjf_p(processes, process_count, &system_config);
            break;
        case 4:
            run_priority_np(processes, process_count, &system_config);
            break;
        case 5:
            run_priority_p(processes, process_count, &system_config);
            break;
        case 6:
            run_rr(processes, process_count, &system_config);
            break;
        case 7:
            compare_algorithms(processes, process_count, &system_config);
            break;
        case 8:
            free(processes);
            return 0;
        default:
            printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
