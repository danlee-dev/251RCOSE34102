#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int time_quantum;  // RR 알고리즘의 타임 퀀텀
    char mode;         // 모드 (y: 랜덤, n: 수동, f: 파일 사용)
    int deadline_miss_info_count; // 데드라인 미스 정보 배열 크기
} Config;

void init_config(Config *config, char mode);

#endif
