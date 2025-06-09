#ifndef UTILS_H
#define UTILS_H

#include "process.h"
#include "queue.h"

typedef enum {
    SORT_BY_PRIORITY,       // for priority
    SORT_BY_REMAINING_TIME, // for SJF
    SORT_BY_DEADLINE,       // for EDF
    SORT_BY_PERIOD          // for RMS
} SortCriteria;

int compare_processes(Process *processes, int pid1, int pid2,
                      SortCriteria criteria);
void sort_queue(Queue *queue, Process *processes, SortCriteria criteria);

void sort_io_operations(Process *p);

void add_io_to_process(Process *p, int io_start, int io_burst);
int get_io_count(Process *p);
int is_time_used(int *used_times, int count, int time);
int has_io_at_progress(Process *p, int progress);
int get_io_burst_at_progress(Process *p, int progress);

#endif
