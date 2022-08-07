def_EHelper(ecall){
    rtl_j(s,isa_raise_intr(INTR_ECALL,s->snpc));
}

static inline rtlreg_t __open_external_intr(){
    rtlreg_t mpie = ((*mstatus >> 7ul) & 1ul);
    return (*mstatus & (~((1ul<<3ul)|(1ul<<7ul)))) | (1ul << 7ul) | (mpie<<3ul);
}
def_EHelper(mret){
    rtl_li(s,mcause,0);
    rtl_li(s,mstatus,__open_external_intr());
    rtl_jr(s,mepc);
}
