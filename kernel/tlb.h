#ifndef TLB_H
#define TLB_H

#include "types.h"

void tlb_flush_all(void);
void tlb_flush_page(uint64 va);
void tlb_flush_asid(int asid);

static inline void
sfence_vma_all(void)
{
  asm volatile("sfence.vma zero, zero");
}

static inline void
sfence_vma_page(uint64 va)
{
  asm volatile("sfence.vma %0, zero" : : "r" (va) : "memory");
}

static inline void
sfence_vma_asid(int asid)
{
  asm volatile("sfence.vma zero, %0" : : "r" (asid) : "memory");
}

struct tlb_stats {
  uint64 flush_all_count;
  uint64 flush_page_count;
  uint64 flush_asid_count;
};

extern struct tlb_stats tlb_stats;

void tlb_init(void);
void tlb_print_stats(void);

#endif
