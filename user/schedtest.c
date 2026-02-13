#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NPROC 8
#define ITERATIONS 1000

void
cpu_bound_task(int priority, int id)
{
  int i, j;
  int count = 0;
  
  setpriority(priority);
  
  printf("Process %d started with priority %d\n", id, priority);
  
  for(i = 0; i < ITERATIONS; i++) {
    for(j = 0; j < 10000; j++) {
      count++;
    }
    
    if(i % 100 == 0) {
      printf("P%d[pri=%d]: iteration %d\n", id, priority, i);
    }
  }
  
  printf("Process %d completed (priority %d)\n", id, priority);
  exit(0);
}

void
test_priority_scheduling(void)
{
  int i, pid;
  int priorities[] = {0, 5, 10, 15, 20, 25, 30, 15};
  
  printf("\n=== Priority Scheduling Test ===\n");
  printf("Creating %d processes with different priorities\n", NPROC);
  printf("Higher priority (lower number) should run first\n\n");
  
  for(i = 0; i < NPROC; i++) {
    pid = fork();
    if(pid < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if(pid == 0) {
      cpu_bound_task(priorities[i], i);
    }
  }
  
  for(i = 0; i < NPROC; i++) {
    wait(0);
  }
  
  printf("\n=== Test Complete ===\n");
}

void
test_aging(void)
{
  int i, pid;
  
  printf("\n=== Aging Test ===\n");
  printf("Creating low-priority process that should get boosted\n\n");
  
  pid = fork();
  if(pid == 0) {
    setpriority(0);
    printf("High priority process running\n");
    for(i = 0; i < 500; i++) {
      if(i % 100 == 0)
        printf("High: %d\n", i);
    }
    exit(0);
  }
  
  pid = fork();
  if(pid == 0) {
    setpriority(30);
    printf("Low priority process running (should be boosted by aging)\n");
    for(i = 0; i < 500; i++) {
      if(i % 100 == 0)
        printf("Low: %d\n", i);
    }
    exit(0);
  }
  
  wait(0);
  wait(0);
  
  printf("\n=== Aging Test Complete ===\n");
}

int
main(int argc, char *argv[])
{
  printf("Scheduler Stress Test\n");
  printf("=====================\n\n");
  
  test_priority_scheduling();
  test_aging();
  
  printf("\nAll tests completed!\n");
  exit(0);
}
