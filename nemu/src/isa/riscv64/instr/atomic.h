def_EHelper(csrrw){
    rtl_mv(s,s0,dsrc2);
    rtl_mv(s,dsrc2,dsrc1);
    rtl_mv(s,ddest,s0);
}

def_EHelper(csrrs){
    rtl_mv(s,s0,dsrc2);
    if(s->isa.instr.i.rs1!=0)
        rtl_and(s,dsrc2,dsrc2,dsrc1);
    rtl_mv(s,ddest,s0);
}

def_EHelper(csrrc){
    rtl_mv(s,s0,dsrc2);
    if(s->isa.instr.i.rs1!=0){
        rtl_inv(s,s1,dsrc1);
        rtl_and(s,dsrc2,dsrc2,s1);
    }
    rtl_mv(s,ddest,s0);
}