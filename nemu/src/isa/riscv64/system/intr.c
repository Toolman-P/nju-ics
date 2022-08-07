#include <isa.h>

#include "../local-include/rtl.h"
#include "../local-include/reg.h"

#define MIE_BIT (((*mstatus)>>3ul)&1ul)
static inline void __close_external_intr(){
  word_t mie = MIE_BIT;
  *mstatus = (*mstatus & ~((1ul<<3ul)|(1ul<<7ul)))|mie;
}

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  *mcause=NO;
  *mepc=epc;

  switch(NO){
    case INTR_TIMER:
      __close_external_intr();
    case INTR_ECALL:
      return *mtvec;
    default:
      return 0;
  }
  
  return 0;
}

word_t isa_query_intr() {
  if(cpu.INTR && MIE_BIT){
    cpu.INTR = false;
    return INTR_TIMER;
  }
  return INTR_EMPTY;
}
