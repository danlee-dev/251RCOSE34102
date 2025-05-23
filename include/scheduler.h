#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "config.h"
#include "evaluation.h"
#include "process.h"

Metrics *run_fcfs(Process *processes, int count, Config *config);
Metrics *run_sjf_np(Process *processes, int count, Config *config);
Metrics *run_sjf_p(Process *processes, int count, Config *config);
Metrics *run_priority_np(Process *processes, int count, Config *config);
Metrics *run_priority_p(Process *processes, int count, Config *config);
Metrics *run_rr(Process *processes, int count, Config *config);

#endif
