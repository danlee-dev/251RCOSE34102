#ifndef SORT_UTILS_H
#define SORT_UTILS_H

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
void insert_sorted(Queue *queue, Process *processes, int pid,
                   SortCriteria criteria);

void insert_by_deadline(Queue *queue, int pid, Process *processes);
int get_earliest_deadline_process(Queue *queue, Process *processes);
void insert_by_period(Queue *queue, int pid, Process *processes);
int get_shortest_period_process(Queue *queue, Process *processes);

void remove_from_queue(Queue *queue, int pid);

#endif
