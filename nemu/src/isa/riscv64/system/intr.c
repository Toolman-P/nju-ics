#include <isa.h>

#include "../local-include/rtl.h"
#include "../local-include/reg.h"

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  *mcause=NO;
  *mepc=epc;

  switch(NO){
    case INTR_ECALL:
      return (word_t)mtvec;
    default:
      return 0;
  }
  
  return 0;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
