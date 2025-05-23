#include <stdio.h>
#include "config.h"

// 설정 초기화
void init_config(Config *config) {
    config->time_quantum = 2;  // 기본값
    config->display_gantt = 1; // 기본값: 간트 차트 표시
}
