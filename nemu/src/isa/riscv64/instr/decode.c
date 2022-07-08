#include "../local-include/reg.h"
#include "cpu/decode.h"
#include <cpu/ifetch.h>
#include <isa-all-instr.h>

def_all_THelper();

static uint32_t get_instr(Decode *s) {
  return s->isa.instr.val;
}

// decode operand helper
#define def_DopHelper(name) \
  void concat(decode_op_, name) (Decode *s, Operand *op, word_t val, bool flag)

static inline def_DopHelper(i) {
  op->imm = val;
}

static inline def_DopHelper(r) {
  bool is_write = flag;
  static word_t zero_null = 0;
  op->preg = (is_write && val == 0) ? &zero_null : &gpr(val);
}

static inline def_DHelper(R) {
  decode_op_r(s,id_src1,s->isa.instr.r.rs1,false);
  decode_op_r(s,id_src2,s->isa.instr.r.rs2,false);
  decode_op_r(s,id_src2,s->isa.instr.r.rd,true);
}

static inline def_DHelper(I) {
  decode_op_r(s, id_src1, s->isa.instr.i.rs1, false);
  rtlreg_t simm = s->isa.instr.i.simm11_0; 

  if(simm & 1<<11)
    simm=c_sext(simm,8);
  decode_op_i(s, id_src2,simm,false);
  decode_op_r(s, id_dest, s->isa.instr.i.rd, true);

}

static inline def_DHelper(U) {
  decode_op_i(s, id_src1, (sword_t)s->isa.instr.u.simm31_12 << 12, false);
  decode_op_r(s, id_dest, s->isa.instr.u.rd, true);
}

static inline def_DHelper(B){
  rtlreg_t simm = (s->isa.instr.b.imm4_1 << 1)  | \
                  (s->isa.instr.b.imm10_5 << 5) | \
                  (s->isa.instr.b.imm11 << 11)  | \
                  (s->isa.instr.b.simm12 << 12);
  if(s->isa.instr.b.simm12)
    simm=c_sext(simm,8);
  decode_op_i(s,id_dest,simm,false);
  decode_op_r(s,id_src1,s->isa.instr.b.rs1,false);
  decode_op_r(s,id_src2,s->isa.instr.b.rs2,false);
}

static inline def_DHelper(S) {
  decode_op_r(s, id_src1, s->isa.instr.s.rs1, false);
  sword_t simm = (s->isa.instr.s.simm11_5 << 5) | s->isa.instr.s.imm4_0;
  decode_op_i(s, id_src2, simm, false);
  decode_op_r(s, id_dest, s->isa.instr.s.rs2, false);
}

static inline def_DHelper(J){
  decode_op_r(s, id_dest, s->isa.instr.j.rd, true);
  
  rtlreg_t simm = (s->isa.instr.j.imm10_1 << 1) | \
                  (s->isa.instr.j.imm11 << 11) | \
                  (s->isa.instr.j.imm19_12 << 12) | \
                  (s->isa.instr.j.simm20 << 20);

  // if(s->isa.instr.j.simm20)
  //   simm = c_sext(simm,8);

  decode_op_i(s,id_src1,simm,false);
  decode_op_r(s,id_dest,s->isa.instr.j.rd, false);
}


def_THelper(ri){
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",addi);
  return EXEC_ID_inv;
}

def_THelper(rr){
  def_INSTR_TAB("??????? ????? ????? 000 ????? ????? ??",add);
  return EXEC_ID_inv;
}

def_THelper(load) {
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??", ld);
  return EXEC_ID_inv;
}

def_THelper(store) {
  def_INSTR_TAB("??????? ????? ????? 011 ????? ????? ??", sd);
  return EXEC_ID_inv;
}

def_THelper(cb){
  return EXEC_ID_inv; 
}

def_THelper(main) {
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01101 11", U     , lui);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00101 11", U     , auipc); // calculation
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00000 11", I     , load); // load

  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01000 11", S     , store); // store
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 00100 11", I     , ri);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 01100 11", R     , rr);
  
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11011 11", J     , jal);  // control
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11001 11", I     , jalr);
  def_INSTR_IDTAB("??????? ????? ????? ??? ????? 11000 11", B     , cb);

  def_INSTR_TAB  ("??????? ????? ????? ??? ????? 11010 11",         nemu_trap); // trap

  return table_inv(s);
};

int isa_fetch_decode(Decode *s) {
  s->isa.instr.val = instr_fetch(&s->snpc, 4);
  int idx = table_main(s);
  return idx;
}
