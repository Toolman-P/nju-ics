#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f)\
              f(auipc)\
              f(addi)\
              f(slli)\
              f(srli)\
              f(srai)\
              f(xori)\
              f(ori)\
              f(andi)\
              f(add)\
              f(sub)\
              f(sll)\
              f(srl)\
              f(sra)\
              f(xor)\
              f(or)\
              f(and)\
              f(beq)\
              f(bne)\
              f(blt)\
              f(bltu)\
              f(bge)\
              f(bgeu)\
              f(lui)\
              f(jal)\
              f(jalr)\
              f(ld)\
              f(sd)\
              f(inv)\
              f(nemu_trap)\

def_all_EXEC_ID();
