#ifndef __RTL_PSEUDO_H__
#define __RTL_PSEUDO_H__
#include "../local-include/reg.h"
#include "c_op.h"
#ifndef __RTL_RTL_H__
#error "Should be only included by <rtl/rtl.h>"
#endif

/* RTL pseudo instructions */

static inline def_rtl(li, rtlreg_t* dest, const rtlreg_t imm) {
  rtl_addi(s,dest,rz,imm);
}

static inline def_rtl(mv, rtlreg_t* dest, const rtlreg_t *src1) {
  rtl_addi(s, dest, src1, 0);
}

static inline def_rtl(not, rtlreg_t *dest, const rtlreg_t* src1) {
  rtl_setrelop(s,RELOP_EQ,dest,src1,rz);
}

static inline def_rtl(neg, rtlreg_t *dest, const rtlreg_t* src1) {
  *dest=c_neg(*src1);
}

static inline def_rtl(sext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
  *dest=c_sext(*src1,width);
}

static inline def_rtl(zext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
  *dest=c_zext(*src1,width);
}

static inline def_rtl(msb, rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- src1[width * 8 - 1]
  *dest=c_srl(c_and(c_sll(1,8*width-1),*src1),8*width-1);
}
#endif
