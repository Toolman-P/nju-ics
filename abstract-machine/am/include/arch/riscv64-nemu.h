#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
  uintptr_t np;
};

#define a0   gpr[10] // calling arguments

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10] // a0
#define GPR3 gpr[11] // a1
#define GPR4 gpr[12] // a2
#define GPRx gpr[10] // ret(a0)

#define MCAUSE_ECALL 11
#define MCAUSE_TIMER 0x8000000000000007ul
#define INTR_YIELD -1

enum{
  USER_MODE = 0,
  KERNEL_MODE = 1
};

#endif
