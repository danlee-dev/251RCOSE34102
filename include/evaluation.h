#ifndef __evaluation_h__
#define __evaluation_h__


typedef struct _evaluation {
    float avg_turnaround_time;
    float avg_waiting_time;
    int idle_time;
    int total_time;
    int finish_time;
} Evaluation;

#endif
