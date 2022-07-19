#ifndef __RISCV64_REG_H__
#define __RISCV64_REG_H__

#include <common.h>

enum{
  REG_SYS=17,
  REG_MSTATUS = 0x300,
  REG_MTVEC = 0x305,
  REG_MEPC = 0x341,
  REG_MCAUSE = 0x342
};

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < 32));
  return idx;
}

static inline int check_csr_idx(int idx){
  IFDEF(CONFIG_RT_CHECK,assert(idx >=0 && idx<4096));
  return idx;
}
#define gpr(idx) (cpu.gpr[check_reg_idx(idx)]._64)
#define csr(idx) (cpu.csr[check_csr_idx(idx)]._64)

#define mtvec &csr(REG_MTVEC)
#define mcause &csr(REG_MCAUSE)
#define mstatus &csr(REG_MSTATUS)
#define mepc &csr(REG_MEPC)
#define rsys &gpr(REG_SYS)

static inline const char* reg_name(int idx, int width) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}


#endif
