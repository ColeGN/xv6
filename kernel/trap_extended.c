#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "vm_extended.h"
#include "tlb.h"

struct trap_stats {
  uint64 syscalls;
  uint64 timer_interrupts;
  uint64 page_faults;
  uint64 external_interrupts;
  uint64 unknown_traps;
} trap_stats;

void
trap_init(void)
{
  trap_stats.syscalls = 0;
  trap_stats.timer_interrupts = 0;
  trap_stats.page_faults = 0;
  trap_stats.external_interrupts = 0;
  trap_stats.unknown_traps = 0;
  printf("Enhanced trap handling initialized\n");
}

void
kerneltrap(void)
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");
  
  if((scause & 0x8000000000000000L) && (scause & 0xff) == 9) {
    which_dev = devintr();
    trap_stats.external_interrupts++;
  } else if(scause == 0x8000000000000005L) {
    which_dev = 2;
    trap_stats.timer_interrupts++;
    if(cpuid() == 0) {
      clockintr();
    }
  } else {
    printf("kerneltrap: scause %p\n", scause);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    trap_stats.unknown_traps++;
    panic("kerneltrap");
  }
  
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();
  
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
usertrap(void)
{
  int which_dev = 0;
  struct proc *p = myproc();
  
  p->trapframe->epc = r_sepc();
  
  uint64 scause = r_scause();
  uint64 stval = r_stval();
  
  if(scause == 8) {
    trap_stats.syscalls++;
    
    if(p->killed)
      exit(-1);
    
    p->trapframe->epc += 4;
    
    intr_on();
    syscall();
    
  } else if((scause & 0x8000000000000000L) && (scause & 0xff) == 9) {
    trap_stats.external_interrupts++;
    which_dev = devintr();
    
  } else if(scause == 0x8000000000000005L) {
    trap_stats.timer_interrupts++;
    which_dev = 2;
    
  } else if(scause == 13 || scause == 15) {
    trap_stats.page_faults++;
    
    struct page_fault_info pf;
    pf.addr = stval;
    pf.type = (scause == 15) ? PF_WRITE : PF_READ;
    pf.scause = scause;
    pf.stval = stval;
    
    if(handle_page_fault(&pf) < 0) {
      printf("usertrap: page fault failed pid=%d addr=%p\n", p->pid, stval);
      p->killed = 1;
    }
    
  } else if(scause == 12) {
    trap_stats.page_faults++;
    
    struct page_fault_info pf;
    pf.addr = stval;
    pf.type = PF_EXEC;
    pf.scause = scause;
    pf.stval = stval;
    
    if(handle_page_fault(&pf) < 0) {
      printf("usertrap: instruction page fault failed pid=%d addr=%p\n", 
             p->pid, stval);
      p->killed = 1;
    }
    
  } else {
    printf("usertrap: unexpected scause %p pid=%d\n", scause, p->pid);
    printf("          sepc=%p stval=%p\n", r_sepc(), stval);
    trap_stats.unknown_traps++;
    p->killed = 1;
  }
  
  if(p->killed)
    exit(-1);
  
  if(which_dev == 2)
    yield();
  
  usertrapret();
}

void
trap_print_stats(void)
{
  printf("\n=== Trap Statistics ===\n");
  printf("System calls: %d\n", trap_stats.syscalls);
  printf("Timer interrupts: %d\n", trap_stats.timer_interrupts);
  printf("Page faults: %d\n", trap_stats.page_faults);
  printf("External interrupts: %d\n", trap_stats.external_interrupts);
  printf("Unknown traps: %d\n", trap_stats.unknown_traps);
  printf("======================\n\n");
}
