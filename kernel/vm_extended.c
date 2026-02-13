#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "vm_extended.h"

struct vm_stats vm_stats;

struct {
  struct spinlock lock;
  int refcount[PHYSTOP / PGSIZE];
} page_refs;

void
vm_init(void)
{
  initlock(&page_refs.lock, "page_refs");
  vm_stats.page_faults = 0;
  vm_stats.cow_faults = 0;
  vm_stats.demand_pages = 0;
  vm_stats.pages_allocated = 0;
  vm_stats.pages_freed = 0;
  printf("Extended VM initialized\n");
}

int
handle_page_fault(struct page_fault_info *pf)
{
  struct proc *p = myproc();
  pagetable_t pagetable = p->pagetable;
  uint64 va = PGROUNDDOWN(pf->addr);
  pte_t *pte;
  
  vm_stats.page_faults++;
  
  if(va >= p->sz || va < 0)
    return -1;
  
  pte = walk(pagetable, va, 0);
  
  if(pte == 0 || (*pte & PTE_V) == 0) {
    return demand_page(pagetable, va);
  }
  
  if((*pte & PTE_COW) && pf->type == PF_WRITE) {
    return cow_handler(pagetable, va);
  }
  
  if(pf->type == PF_WRITE && (*pte & PTE_W) == 0) {
    return -1;
  }
  
  return -1;
}

int
demand_page(pagetable_t pagetable, uint64 va)
{
  char *mem;
  uint64 pa;
  
  mem = kalloc();
  if(mem == 0) {
    return -1;
  }
  
  memset(mem, 0, PGSIZE);
  pa = (uint64)mem;
  
  if(mappages(pagetable, va, PGSIZE, pa, 
              PTE_R | PTE_W | PTE_X | PTE_U) != 0) {
    kfree(mem);
    return -1;
  }
  
  sfence_vma_page(va);
  
  vm_stats.demand_pages++;
  vm_stats.pages_allocated++;
  
  return 0;
}

int
cow_handler(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  uint64 pa, new_pa;
  uint flags;
  char *mem;
  
  pte = walk(pagetable, va, 0);
  if(pte == 0)
    return -1;
  
  pa = PTE2PA(*pte);
  flags = PTE_FLAGS(*pte);
  
  if(page_getref((void*)pa) == 1) {
    *pte = PA2PTE(pa) | (flags & ~PTE_COW) | PTE_W;
  } else {
    mem = kalloc();
    if(mem == 0)
      return -1;
    
    memmove(mem, (char*)pa, PGSIZE);
    new_pa = (uint64)mem;
    
    *pte = PA2PTE(new_pa) | (flags & ~PTE_COW) | PTE_W;
    
    page_decref((void*)pa);
    page_incref(mem);
    
    vm_stats.pages_allocated++;
  }
  
  sfence_vma_page(va);
  
  vm_stats.cow_faults++;
  
  return 0;
}

int
setup_cow_page(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  
  pte = walk(pagetable, va, 0);
  if(pte == 0 || (*pte & PTE_V) == 0)
    return -1;
  
  *pte = (*pte & ~PTE_W) | PTE_COW;
  
  uint64 pa = PTE2PA(*pte);
  page_incref((void*)pa);
  
  return 0;
}

void
page_incref(void *pa)
{
  int idx = (uint64)pa / PGSIZE;
  acquire(&page_refs.lock);
  page_refs.refcount[idx]++;
  release(&page_refs.lock);
}

void
page_decref(void *pa)
{
  int idx = (uint64)pa / PGSIZE;
  acquire(&page_refs.lock);
  if(page_refs.refcount[idx] > 0)
    page_refs.refcount[idx]--;
  if(page_refs.refcount[idx] == 0) {
    kfree(pa);
    vm_stats.pages_freed++;
  }
  release(&page_refs.lock);
}

int
page_getref(void *pa)
{
  int idx = (uint64)pa / PGSIZE;
  int ref;
  acquire(&page_refs.lock);
  ref = page_refs.refcount[idx];
  release(&page_refs.lock);
  return ref;
}

void
vm_print_stats(void)
{
  printf("\n=== VM Statistics ===\n");
  printf("Page faults: %d\n", vm_stats.page_faults);
  printf("COW faults: %d\n", vm_stats.cow_faults);
  printf("Demand pages: %d\n", vm_stats.demand_pages);
  printf("Pages allocated: %d\n", vm_stats.pages_allocated);
  printf("Pages freed: %d\n", vm_stats.pages_freed);
  printf("====================\n\n");
}
