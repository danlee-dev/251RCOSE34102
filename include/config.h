#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int time_quantum;  // RR 알고리즘의 타임 퀀텀
    int display_gantt; // 간트 차트 표시 여부
} Config;

void init_config(Config *config);
void set_config(Config *config);

#endif
