#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NPROC 6
#define PGSIZE 4096

void
memory_intensive(int id)
{
  char *p;
  int i, j;
  
  printf("Memory process %d started\n", id);
  
  for(i = 0; i < 5; i++) {
    p = sbrk(PGSIZE * 2);
    if(p == (char*)-1) {
      printf("sbrk failed in process %d\n", id);
      exit(1);
    }
    
    for(j = 0; j < PGSIZE * 2; j += 512) {
      p[j] = 'A' + id;
    }
    
    printf("M%d: allocated and wrote %d pages\n", id, i);
    
    yield();
  }
  
  printf("Memory process %d completed\n", id);
  exit(0);
}

void
cpu_intensive(int id, int priority)
{
  int i, j;
  int sum = 0;
  
  setpriority(priority);
  printf("CPU process %d started (priority %d)\n", id, priority);
  
  for(i = 0; i < 500; i++) {
    for(j = 0; j < 1000; j++) {
      sum += j;
    }
    
    if(i % 100 == 0) {
      printf("C%d[%d]: iteration %d\n", id, priority, i);
    }
  }
  
  printf("CPU process %d completed\n", id);
  exit(0);
}

void
fork_intensive(int id)
{
  int i, pid;
  
  printf("Fork process %d started\n", id);
  
  for(i = 0; i < 3; i++) {
    pid = fork();
    if(pid < 0) {
      printf("fork failed in process %d\n", id);
      exit(1);
    }
    
    if(pid == 0) {
      printf("  F%d child %d\n", id, i);
      
      int j, sum = 0;
      for(j = 0; j < 500; j++) {
        sum += j;
      }
      
      exit(0);
    }
  }
  
  for(i = 0; i < 3; i++) {
    wait(0);
  }
  
  printf("Fork process %d completed\n", id);
  exit(0);
}

void
mixed_workload(int id)
{
  char *p;
  int i, j;
  int priority = (id * 5) % 30;
  
  setpriority(priority);
  printf("Mixed process %d started (priority %d)\n", id, priority);
  
  for(i = 0; i < 3; i++) {
    p = sbrk(PGSIZE);
    if(p != (char*)-1) {
      p[0] = 'M';
      p[PGSIZE-1] = 'X';
    }
    
    int sum = 0;
    for(j = 0; j < 1000; j++) {
      sum += j;
    }
    
    int pid = fork();
    if(pid == 0) {
      printf("  Mixed %d child\n", id);
      exit(0);
    } else if(pid > 0) {
      wait(0);
    }
    
    printf("X%d: round %d complete\n", id, i);
    yield();
  }
  
  printf("Mixed process %d completed\n", id);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int i, pid;
  
  printf("Combined Stress Test\n");
  printf("====================\n");
  printf("Testing scheduler, memory, and concurrency\n\n");
  
  for(i = 0; i < 2; i++) {
    pid = fork();
    if(pid == 0) {
      memory_intensive(i);
    }
  }
  
  for(i = 0; i < 2; i++) {
    pid = fork();
    if(pid == 0) {
      cpu_intensive(i, i * 10);
    }
  }
  
  pid = fork();
  if(pid == 0) {
    fork_intensive(0);
  }
  
  pid = fork();
  if(pid == 0) {
    mixed_workload(0);
  }
  
  printf("\nParent waiting for all processes...\n");
  for(i = 0; i < 6; i++) {
    wait(0);
  }
  
  printf("\n=== All Stress Tests Completed ===\n");
  printf("Check kernel logs for statistics\n");
  
  exit(0);
}
