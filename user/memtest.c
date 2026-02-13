#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096
#define NPAGES 10

void
test_demand_paging(void)
{
  char *pages[NPAGES];
  int i, j;
  
  printf("\n=== Demand Paging Test ===\n");
  printf("Allocating %d pages on demand\n", NPAGES);
  
  for(i = 0; i < NPAGES; i++) {
    pages[i] = sbrk(PGSIZE);
    if(pages[i] == (char*)-1) {
      printf("sbrk failed\n");
      exit(1);
    }
    printf("Allocated page %d at %p\n", i, pages[i]);
    
    pages[i][0] = 'A' + i;
    pages[i][PGSIZE-1] = 'Z' - i;
    
    printf("  Wrote to page %d\n", i);
  }
  
  printf("\nVerifying data...\n");
  for(i = 0; i < NPAGES; i++) {
    if(pages[i][0] != 'A' + i || pages[i][PGSIZE-1] != 'Z' - i) {
      printf("Data corruption on page %d!\n", i);
      exit(1);
    }
  }
  printf("All pages verified successfully\n");
  
  printf("=== Demand Paging Test Complete ===\n");
}

void
test_page_faults(void)
{
  char *p;
  int i;
  
  printf("\n=== Page Fault Test ===\n");
  printf("Triggering intentional page faults\n");
  
  p = sbrk(PGSIZE * 5);
  if(p == (char*)-1) {
    printf("sbrk failed\n");
    exit(1);
  }
  
  printf("Accessing pages in random order...\n");
  p[PGSIZE * 4] = 'A';
  printf("  Accessed page 4\n");
  
  p[PGSIZE * 1] = 'B';
  printf("  Accessed page 1\n");
  
  p[PGSIZE * 3] = 'C';
  printf("  Accessed page 3\n");
  
  p[PGSIZE * 0] = 'D';
  printf("  Accessed page 0\n");
  
  p[PGSIZE * 2] = 'E';
  printf("  Accessed page 2\n");
  
  if(p[PGSIZE * 4] != 'A' || p[PGSIZE * 1] != 'B' ||
     p[PGSIZE * 3] != 'C' || p[PGSIZE * 0] != 'D' ||
     p[PGSIZE * 2] != 'E') {
    printf("Data verification failed!\n");
    exit(1);
  }
  
  printf("All page faults handled correctly\n");
  printf("=== Page Fault Test Complete ===\n");
}

void
test_cow_fork(void)
{
  char *p;
  int pid;
  
  printf("\n=== Copy-on-Write Fork Test ===\n");
  
  p = sbrk(PGSIZE * 2);
  if(p == (char*)-1) {
    printf("sbrk failed\n");
    exit(1);
  }
  
  p[0] = 'P';
  p[PGSIZE] = 'Q';
  printf("Parent initialized memory: %c, %c\n", p[0], p[PGSIZE]);
  
  pid = fork();
  if(pid < 0) {
    printf("fork failed\n");
    exit(1);
  }
  
  if(pid == 0) {
    printf("Child reading: %c, %c (should share pages)\n", p[0], p[PGSIZE]);
    
    printf("Child writing (should trigger COW)...\n");
    p[0] = 'C';
    p[PGSIZE] = 'D';
    
    printf("Child after write: %c, %c\n", p[0], p[PGSIZE]);
    exit(0);
  } else {
    wait(0);
    printf("Parent after child exit: %c, %c (should be unchanged)\n", 
           p[0], p[PGSIZE]);
    
    if(p[0] != 'P' || p[PGSIZE] != 'Q') {
      printf("COW failed - parent memory was modified!\n");
      exit(1);
    }
    
    printf("COW working correctly\n");
  }
  
  printf("=== COW Test Complete ===\n");
}

int
main(int argc, char *argv[])
{
  printf("Memory and Page Fault Test\n");
  printf("===========================\n");
  
  test_demand_paging();
  test_page_faults();
  test_cow_fork();
  
  printf("\nAll memory tests completed!\n");
  exit(0);
}
