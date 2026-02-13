#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "proc.h"

#define PRIORITY_MAX 0
#define PRIORITY_MIN 31
#define PRIORITY_DEFAULT 15

#define AGING_THRESHOLD 100
#define AGING_BOOST 1

struct sched_stats {
  uint64 context_switches;
  uint64 total_wait_time;
  uint64 total_run_time;
};

struct sched_info {
  int priority;
  int base_priority;
  uint64 wait_ticks;
  uint64 run_ticks;
  uint64 last_scheduled;
};

void scheduler_init(void);
void scheduler(void) __attribute__((noreturn));
void sched_yield(void);
void sched_setpriority(struct proc *p, int priority);
int sched_getpriority(struct proc *p);
void sched_update_stats(struct proc *p);
void sched_age_processes(void);
void sched_debug_print(void);

#endif
