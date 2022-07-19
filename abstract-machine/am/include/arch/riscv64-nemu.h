#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10] // a0
#define GPR3 gpr[11] // a1
#define GPR4 gpr[12] // a2
#define GPR5 gpr[13] // a3
#define GPR6 gpr[14] // a4
#define GPR7 gpr[15] // a5


#define MCAUSE_ECALL 11
#define INTR_YIELD -1

#endif
