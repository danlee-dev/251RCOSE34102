#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "evaluation.h"
#include "process.h"
#include "queue.h"
#include "scheduling.h"

int main() {
    printf("====================================================\n");
    printf("          CPU Scheduling Simulator\n");
    printf("====================================================\n\n");

    // Config
    CreateProcesses();
    Config();

    // Initialize evaluation results tracking

    // Create processes with random attributes

    // Run different scheduling algorithms
    RunFCFS();
    RunNonPreemptiveSJF();
    RunPreemptiveSJF();
    RunNonPreemptivePriority();
    RunPreemptivePriority();
    RunRoundRobin();

    // Evaluate and display results
    EvaluateResults();

    // Free allocated resources
    FreeResources();

    printf("\n====================================================\n");
    printf("          Simulation Completed\n");
    printf("====================================================\n");

    return 0;
}
