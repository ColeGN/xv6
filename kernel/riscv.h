// RISC-V definitions for xv6-extended
// Add these to your existing riscv.h

// Page table entry flags
#define PTE_V (1L << 0) // valid
#define PTE_R (1L << 1) // readable
#define PTE_W (1L << 2) // writable
#define PTE_X (1L << 3) // executable
#define PTE_U (1L << 4) // user accessible
#define PTE_G (1L << 5) // global
#define PTE_A (1L << 6) // accessed
#define PTE_D (1L << 7) // dirty
#define PTE_COW (1L << 8) // copy-on-write (custom flag)

// Extract physical page number from PTE
#define PTE2PA(pte) (((pte) >> 10) << 12)
#define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)
#define PTE_FLAGS(pte) ((pte) & 0x3FF)

// Page size
#define PGSIZE 4096
#define PGSHIFT 12
#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

// Supervisor Status Register (sstatus)
#define SSTATUS_SPP (1L << 8)  // Previous mode (1=Supervisor, 0=User)
#define SSTATUS_SPIE (1L << 5) // Previous interrupt enable
#define SSTATUS_UPIE (1L << 4) // User previous interrupt enable
#define SSTATUS_SIE (1L << 1)  // Supervisor interrupt enable
#define SSTATUS_UIE (1L << 0)  // User interrupt enable

// Supervisor Interrupt Enable (sie)
#define SIE_SEIE (1L << 9) // external
#define SIE_STIE (1L << 5) // timer
#define SIE_SSIE (1L << 1) // software

// Machine-mode Interrupt Enable (mie)
#define MIE_MEIE (1L << 11) // external
#define MIE_MTIE (1L << 7)  // timer
#define MIE_MSIE (1L << 3)  // software

// Read/write CSR functions
static inline uint64
r_scause()
{
  uint64 x;
  asm volatile("csrr %0, scause" : "=r" (x) );
  return x;
}

static inline uint64
r_sepc()
{
  uint64 x;
  asm volatile("csrr %0, sepc" : "=r" (x) );
  return x;
}

static inline void
w_sepc(uint64 x)
{
  asm volatile("csrw sepc, %0" : : "r" (x));
}

static inline uint64
r_stval()
{
  uint64 x;
  asm volatile("csrr %0, stval" : "=r" (x) );
  return x;
}

static inline uint64
r_sstatus()
{
  uint64 x;
  asm volatile("csrr %0, sstatus" : "=r" (x) );
  return x;
}

static inline void
w_sstatus(uint64 x)
{
  asm volatile("csrw sstatus, %0" : : "r" (x));
}

static inline uint64
r_satp()
{
  uint64 x;
  asm volatile("csrr %0, satp" : "=r" (x) );
  return x;
}

static inline void
w_satp(uint64 x)
{
  asm volatile("csrw satp, %0" : : "r" (x));
}
