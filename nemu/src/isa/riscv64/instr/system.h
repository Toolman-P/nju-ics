def_EHelper(ecall){
    rtl_jr(s,(rtlreg_t *)isa_raise_intr(INTR_ECALL,s->snpc));
}

def_EHelper(mret){
    rtl_li(s,mcause,0);
    rtl_jr(s,mepc);
}
