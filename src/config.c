#include "config.h"
#include <stdio.h>
#include <stdlib.h>
// 설정 초기화
void init_config(Config *config, char mode) {
    if (mode == 'y') {
        config->time_quantum = rand() % 3 + 1; // 1~3 사이 타임 퀀텀
    } else {
        printf("Enter the time quantum: ");
        scanf("%d", &config->time_quantum);
    }
}
