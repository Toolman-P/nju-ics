def_EHelper(jal){
  rtl_li(s,ddest,s->snpc);
  rtl_j(s,s->pc+id_src1->simm);
}

def_EHelper(jalr){
  rtl_li(s,ddest,s->snpc);
  rtl_addi(s,s0,dsrc1,id_src2->simm);
  rtl_andi(s,s1,s0,0xFFFFFFFE); 
  rtl_jr(s,s1);
}

