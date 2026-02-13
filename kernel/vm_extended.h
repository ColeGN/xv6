#ifndef VM_EXTENDED_H
#define VM_EXTENDED_H

#include "types.h"
#include "riscv.h"

#define PF_READ  0
#define PF_WRITE 1
#define PF_EXEC  2

#define PTE_COW (1L << 8)

struct page_fault_info {
  uint64 addr;
  int type;
  uint64 scause;
  uint64 stval;
};

struct vm_stats {
  uint64 page_faults;
  uint64 cow_faults;
  uint64 demand_pages;
  uint64 pages_allocated;
  uint64 pages_freed;
};

extern struct vm_stats vm_stats;

void vm_init(void);
int handle_page_fault(struct page_fault_info *pf);
int demand_page(pagetable_t pagetable, uint64 va);
int cow_handler(pagetable_t pagetable, uint64 va);
int setup_cow_page(pagetable_t pagetable, uint64 va);
void vm_print_stats(void);

void page_incref(void *pa);
void page_decref(void *pa);
int page_getref(void *pa);

#endif
