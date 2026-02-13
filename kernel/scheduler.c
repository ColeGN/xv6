#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "scheduler.h"

struct sched_stats global_stats;

void
scheduler_init(void)
{
  global_stats.context_switches = 0;
  global_stats.total_wait_time = 0;
  global_stats.total_run_time = 0;
  printf("Priority scheduler initialized\n");
}

static struct proc*
find_highest_priority(void)
{
  struct proc *p;
  struct proc *best = 0;
  int best_priority = PRIORITY_MIN + 1;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    
    if(p->state == RUNNABLE) {
      int priority = p->sched_info.priority;
      
      if(priority < best_priority || 
         (priority == best_priority && best && 
          p->sched_info.wait_ticks > best->sched_info.wait_ticks)) {
        if(best)
          release(&best->lock);
        best = p;
        best_priority = priority;
        continue;
      }
    }
    
    release(&p->lock);
  }
  
  return best;
}

void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  
  c->proc = 0;
  
  for(;;) {
    intr_on();
    
    sched_age_processes();
    
    p = find_highest_priority();
    
    if(p) {
      p->sched_info.last_scheduled = ticks;
      p->sched_info.wait_ticks = 0;
      
      p->state = RUNNING;
      c->proc = p;
      
      global_stats.context_switches++;
      
      swtch(&c->context, &p->context);
      
      c->proc = 0;
      release(&p->lock);
    }
  }
}

void
sched_age_processes(void)
{
  struct proc *p;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    
    if(p->state == RUNNABLE) {
      p->sched_info.wait_ticks++;
      
      if(p->sched_info.wait_ticks >= AGING_THRESHOLD) {
        if(p->sched_info.priority > PRIORITY_MAX) {
          p->sched_info.priority -= AGING_BOOST;
        }
        p->sched_info.wait_ticks = 0;
      }
    }
    
    release(&p->lock);
  }
}

void
sched_setpriority(struct proc *p, int priority)
{
  if(priority < PRIORITY_MAX)
    priority = PRIORITY_MAX;
  if(priority > PRIORITY_MIN)
    priority = PRIORITY_MIN;
    
  p->sched_info.priority = priority;
  p->sched_info.base_priority = priority;
}

int
sched_getpriority(struct proc *p)
{
  return p->sched_info.priority;
}

void
sched_update_stats(struct proc *p)
{
  p->sched_info.run_ticks++;
  global_stats.total_run_time++;
}

void
sched_debug_print(void)
{
  struct proc *p;
  
  printf("\n=== Scheduler State ===\n");
  printf("Context switches: %d\n", global_stats.context_switches);
  printf("Total run time: %d\n", global_stats.total_run_time);
  
  printf("\nProcess Table:\n");
  printf("PID\tSTATE\t\tPRIO\tWAIT\tRUN\n");
  
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state != UNUSED) {
      printf("%d\t%s\t%d\t%d\t%d\n",
             p->pid,
             p->state == RUNNING ? "RUNNING" :
             p->state == RUNNABLE ? "RUNNABLE" :
             p->state == SLEEPING ? "SLEEPING" : "OTHER",
             p->sched_info.priority,
             p->sched_info.wait_ticks,
             p->sched_info.run_ticks);
    }
    release(&p->lock);
  }
  printf("======================\n\n");
}
