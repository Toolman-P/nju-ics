#define def_exec_compute_imm(name) \
  def_EHelper(concat(name,i)){ \
    concat3(rtl_,name,i)(s,ddest,dsrc1,id_src2->simm); \
  }
  
#define def_exec_compute_imm_w(name) \
  def_EHelper(concat(name,iw)){ \
    concat3(rtl_,name,iw)(s,ddest,dsrc1,id_src2->simm); \
  }

#define def_exec_compute_reg(name) \
  def_EHelper(name){ \
    concat(rtl_,name)(s,ddest,dsrc1,dsrc2);\
  }

#define def_exec_compute_reg_w(name)\
  def_EHelper(concat(name,w)){ \
    concat3(rtl_,name,w)(s,ddest,dsrc1,dsrc2);\
  }

#define def_exec_compute_reg_imm(name) \
  def_exec_compute_reg(name) \
  def_exec_compute_imm(name)
 
#define def_exec_compute_reg_imm_w(name) \
  def_exec_compute_reg_w(name) \
  def_exec_compute_imm_w(name) 


def_exec_compute_reg_imm(add)
def_exec_compute_reg_imm_w(add)

def_exec_compute_reg_imm(sll)
def_exec_compute_reg_imm_w(sll)

def_exec_compute_reg_imm(srl)
def_exec_compute_reg_imm_w(srl)

def_exec_compute_reg_imm(sra)
def_exec_compute_reg_imm_w(sra)

def_exec_compute_reg_imm(xor)
def_exec_compute_reg_imm(and)
def_exec_compute_reg_imm(or)

def_exec_compute_reg(sub)
def_exec_compute_reg_w(sub)

def_EHelper(auipc) {
  rtl_li(s, ddest, id_src1->imm + s->pc);
}

def_EHelper(slti){
  rtl_setrelopi(s,RELOP_LT,ddest,dsrc1,id_src2->simm);
}

def_EHelper(sltiu){
  rtl_setrelopi(s,RELOP_LTU,ddest,dsrc1,id_src2->simm);
}

def_EHelper(slt){
  rtl_setrelop(s,RELOP_LT,ddest,dsrc1,dsrc2);
}

def_EHelper(sltu){
  rtl_setrelop(s,RELOP_LTU,ddest,dsrc1,dsrc2);
}

def_EHelper(mul){
  rtl_mulu_lo(s,ddest,dsrc1,dsrc2);
}

def_EHelper(mulh){
  rtl_muls_hi(s,ddest,dsrc1,dsrc2);
}

def_EHelper(mulhsu){
  rtl_mulu_hi(s,ddest,dsrc1,dsrc2);
}

def_EHelper(mulhu){
  rtl_mulu_hi(s,ddest,dsrc1,dsrc2);
}

def_EHelper(div){
  rtl_divs_q(s,ddest,dsrc1,dsrc2);
}

def_EHelper(divu){
  rtl_divu_q(s,ddest,dsrc1,dsrc2);
}

def_EHelper(rem){
  rtl_divs_r(s,ddest,dsrc1,dsrc2);
}

def_EHelper(remu){
  rtl_divu_r(s,ddest,dsrc1,dsrc2);
}

def_exec_compute_reg_w(mul)
def_exec_compute_reg_w(div)
def_exec_compute_reg_w(divu)
def_exec_compute_reg_w(rem)
def_exec_compute_reg_w(remu)