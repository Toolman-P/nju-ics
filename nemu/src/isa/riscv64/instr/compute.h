def_EHelper(auipc) {
  rtl_li(s, ddest, id_src1->imm + s->pc);
}

def_EHelper(addi){
  rtl_addi(s,ddest,dsrc1,id_src2->simm);
}

def_EHelper(add){
  rtl_add(s,ddest,dsrc1,dsrc2);
}
