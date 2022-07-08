#define relop_beq RELOP_EQ
#define relop_bne RELOP_NE
#define relop_blt RELOP_LT
#define relop_bltu RELOP_LTU
#define relop_bge RELOP_GE
#define relop_bgeu RELOP_GEU

#define MAP_RELOP(bn) concat(relop_,bn)
#define EXPAND_RELOP(bn) MAP_RELOP(bn)

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

#define def_conditional_branch(name) \
  def_EHelper(name){ \
    rtl_jrelop(s,EXPAND_RELOP(name),dsrc1,dsrc2,s->pc+id_dest->simm);\
  }

def_conditional_branch(beq)
def_conditional_branch(bne)
def_conditional_branch(blt)
def_conditional_branch(bltu)
def_conditional_branch(bge)
def_conditional_branch(bgeu)
