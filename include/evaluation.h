#ifndef EVALUATION_H
#define EVALUATION_H

#include "config.h"
#include "process.h"

// 간트차트 엔트리 구조체
typedef struct {
    int time_start;
    int time_end;
    int process_id;
    char status[10]; // "RUN", "IDLE", "IO" 등
} GanttEntry;

// 간트차트 구조체
typedef struct {
    GanttEntry *entries;
    int count;
    int capacity;
} GanttChart;

// 성능 지표 구조체
typedef struct {
    int total_time;
    int idle_time;
} Metrics;

void display_metrics(Process *processes, int count, int total_time,
                     int idle_time);
void compare_algorithms(Process *processes, int count, Config *config);

// 간트차트 관련 함수들
void add_gantt_entry(GanttChart *gantt, int start, int end, int pid,
                     const char *status);
void display_performance_summary(Process *processes, int count, int total_time,
                                 int idle_time);
void display_process_table(Process *processes, int count);

// 실제 사용되는 간트차트 관련 함수들
GanttChart *consolidate_gantt_chart(GanttChart *original);
void display_gantt_chart(GanttChart *gantt, const char *algorithm_name);
void display_scheduling_results(Process *processes, int count,
                                GanttChart *gantt, int total_time,
                                int idle_time, const char *algorithm_name);

#endif
