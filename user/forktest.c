#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXPROC 20

void
simple_fork_test(void)
{
  int pid;
  
  printf("\n=== Simple Fork Test ===\n");
  
  pid = fork();
  if(pid < 0) {
    printf("fork failed\n");
    exit(1);
  }
  
  if(pid == 0) {
    printf("Child process (pid=%d)\n", getpid());
    exit(0);
  } else {
    printf("Parent process (pid=%d) created child (pid=%d)\n", 
           getpid(), pid);
    wait(0);
    printf("Child exited\n");
  }
  
  printf("=== Simple Fork Test Complete ===\n");
}

void
many_forks_test(void)
{
  int i, pid;
  int n = 10;
  
  printf("\n=== Many Forks Test ===\n");
  printf("Creating %d child processes\n", n);
  
  for(i = 0; i < n; i++) {
    pid = fork();
    if(pid < 0) {
      printf("fork %d failed\n", i);
      exit(1);
    }
    
    if(pid == 0) {
      printf("Child %d (pid=%d) running\n", i, getpid());
      
      int j, sum = 0;
      for(j = 0; j < 1000; j++) {
        sum += j;
      }
      
      printf("Child %d done (sum=%d)\n", i, sum);
      exit(0);
    }
  }
  
  printf("Parent waiting for children...\n");
  for(i = 0; i < n; i++) {
    wait(0);
  }
  
  printf("All children completed\n");
  printf("=== Many Forks Test Complete ===\n");
}

void
nested_fork_test(void)
{
  int pid1, pid2;
  
  printf("\n=== Nested Fork Test ===\n");
  
  pid1 = fork();
  if(pid1 < 0) {
    printf("fork failed\n");
    exit(1);
  }
  
  if(pid1 == 0) {
    printf("First child (pid=%d)\n", getpid());
    
    pid2 = fork();
    if(pid2 < 0) {
      printf("nested fork failed\n");
      exit(1);
    }
    
    if(pid2 == 0) {
      printf("  Grandchild (pid=%d)\n", getpid());
      exit(0);
    } else {
      printf("  First child waiting for grandchild\n");
      wait(0);
      printf("  Grandchild exited\n");
      exit(0);
    }
  } else {
    printf("Parent waiting for first child\n");
    wait(0);
    printf("First child exited\n");
  }
  
  printf("=== Nested Fork Test Complete ===\n");
}

void
context_switch_test(void)
{
  int i, pid;
  int n = 5;
  
  printf("\n=== Context Switch Test ===\n");
  printf("Creating %d processes to stress context switching\n", n);
  
  for(i = 0; i < n; i++) {
    pid = fork();
    if(pid < 0) {
      printf("fork failed\n");
      exit(1);
    }
    
    if(pid == 0) {
      int j;
      for(j = 0; j < 100; j++) {
        if(j % 10 == 0)
          printf("P%d: %d\n", i, j);
        yield();
      }
      exit(0);
    }
  }
  
  for(i = 0; i < n; i++) {
    wait(0);
  }
  
  printf("=== Context Switch Test Complete ===\n");
}

int
main(int argc, char *argv[])
{
  printf("Fork and Context Switch Test\n");
  printf("============================\n");
  
  simple_fork_test();
  many_forks_test();
  nested_fork_test();
  context_switch_test();
  
  printf("\nAll fork tests completed!\n");
  exit(0);
}
