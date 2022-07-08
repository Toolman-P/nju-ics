#define def_exec_compute_imm(name) \
  def_EHelper(name ## i){ \
    concat(rtl_,name ## i)(s,ddest,dsrc1,id_src2->simm); \
  }

#define def_exec_compute_reg(name) \
  def_EHelper(name){ \
    concat(rtl_,name)(s,ddest,dsrc1,dsrc2);\
  }

#define def_exec_compute_reg_imm(name) \
  def_exec_compute_reg(name) \
  def_exec_compute_imm(name) \

def_EHelper(auipc) {
  rtl_li(s, ddest, id_src1->imm + s->pc);
}

def_exec_compute_reg_imm(add)
def_exec_compute_reg_imm(sll)
def_exec_compute_reg_imm(srl)
def_exec_compute_reg_imm(sra)
def_exec_compute_reg_imm(xor)
def_exec_compute_reg_imm(and)
def_exec_compute_reg_imm(or)
def_exec_compute_reg(sub)
