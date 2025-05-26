#include "config.h"
#include <stdio.h>
#include <stdlib.h>
// 설정 초기화
void init_config(Config *config, char mode) {
    if (mode == 'y') {
        config->time_quantum = rand() % 3 + 1;
        config->mode = 'y';
    } else if (mode == 'n') {
        printf("Enter the time quantum: ");
        scanf("%d", &config->time_quantum);
        config->mode = 'n';
    } else if (mode == 'f') {
        printf("Enter the time quantum: ");
        scanf("%d", &config->time_quantum);
        config->mode = 'f';
    } else {
        printf("Invalid mode\n");
        exit(1);
    }
}
