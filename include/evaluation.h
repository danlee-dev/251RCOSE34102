#ifndef EVALUATION_H
#define EVALUATION_H

#include "config.h"
#include "process.h"

typedef struct {
    int time_start;
    int time_end;
    int process_id;
    char status[10]; // "RUN", "IDLE"
} GanttEntry;

typedef struct {
    GanttEntry *entries;
    int count;
    int capacity;
} GanttChart;

typedef struct {
    int total_time;
    int idle_time;
} Metrics;

typedef struct {
    char name[32];
    float avg_wait_time;
    float avg_turnaround_time;
    float cpu_utilization;
    float throughput;
    int total_time;
    int missed_deadlines;
} AlgorithmMetrics;

void print_thin_emphasized_header(const char *title, int width);
void print_emphasized_header(const char *title, int width);
void print_section_divider(const char *title, int width);
void print_section_end(int width);

void display_metrics(Process *processes, int count, int total_time,
                     int idle_time);
void compare_algorithms(Process *processes, int count, Config *config);

void display_performance_summary(Process *processes, int count, int total_time,
                                 int idle_time);
void display_process_table(Process *processes, int count);
void display_io_statistics(Process *processes, int count);

GanttChart *consolidate_gantt_chart(GanttChart *original);
void add_gantt_entry(GanttChart *gantt, int start, int end, int pid,
                     const char *status);
void display_gantt_chart(GanttChart *gantt, const char *algorithm_name);
void display_scheduling_results(Process *processes, int count,
                                GanttChart *gantt, int total_time,
                                int idle_time, const char *algorithm_name);
void save_processes_to_file(Process *processes, int count);
void list_saved_process_files();
void show_file_format_example();
void show_file_management_menu();

#endif
