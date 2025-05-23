#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int time_quantum;  // RR 알고리즘의 타임 퀀텀
} Config;

void init_config(Config *config, char mode);

#endif
