#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "config.h"
#include "evaluation.h"
#include "process.h"
#include "queue.h"

Metrics *run_fcfs(Process *processes, int count);
Metrics *run_sjf_np(Process *processes, int count);
Metrics *run_sjf_p(Process *processes, int count);
Metrics *run_priority_np(Process *processes, int count);
Metrics *run_priority_p(Process *processes, int count);
Metrics *run_rr(Process *processes, int count, Config *config);
Metrics *run_priority_with_aging(Process *processes, int count);
Metrics *run_edf(Process *processes, int count, Config *config, int max_time);
Metrics *run_rms(Process *processes, int count, Config *config, int max_time);

#endif
