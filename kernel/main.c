#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;
extern char _entry;
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  // legacy code from start.c (machine mode code)
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
  w_stimecmp(r_time() + 1000000);
  for (int i = 0; i < NCPU; i++) {
    asm volatile("mv a0, %0" :: "r" (i) : "a0");
    asm volatile("mv a1, %0" :: "r" (&_entry) : "a1");
    asm volatile("li a2, 0" ::: "a2");
    asm volatile("li a6, 0" ::: "a6");
    asm volatile("li a7, 0x48534d" ::: "a7");
    asm volatile("ecall");
  }

  if(cpuid() == 0){
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();         // physical page allocator
    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    procinit();      // process table
    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
    virtio_disk_init(); // emulated hard disk
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
