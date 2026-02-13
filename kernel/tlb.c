#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "tlb.h"

struct tlb_stats tlb_stats;

void
tlb_init(void)
{
  tlb_stats.flush_all_count = 0;
  tlb_stats.flush_page_count = 0;
  tlb_stats.flush_asid_count = 0;
  printf("TLB management initialized\n");
}

void
tlb_flush_all(void)
{
  sfence_vma_all();
  tlb_stats.flush_all_count++;
}

void
tlb_flush_page(uint64 va)
{
  sfence_vma_page(va);
  tlb_stats.flush_page_count++;
}

void
tlb_flush_asid(int asid)
{
  sfence_vma_asid(asid);
  tlb_stats.flush_asid_count++;
}

void
tlb_print_stats(void)
{
  printf("\n=== TLB Statistics ===\n");
  printf("Full flushes: %d\n", tlb_stats.flush_all_count);
  printf("Page flushes: %d\n", tlb_stats.flush_page_count);
  printf("ASID flushes: %d\n", tlb_stats.flush_asid_count);
  printf("=====================\n\n");
}
