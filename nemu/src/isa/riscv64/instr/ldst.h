#include "rtl/rtl.h"
#define LEN_d 8
#define LEN_w 4
#define LEN_h 2
#define LEN_b 1
#define ldu ld
#define sdu sd

#define MAP_LEN(type) concat(LEN_,type)
#define EXPAND_LEN(type) MAP_LEN(type)

#define def_exec_unsigned_load(type) \
  def_EHelper(concat3(l,type,u)){ \
    rtl_lm(s,ddest,dsrc1,id_src2->simm,EXPAND_LEN(type));\
  }

#define def_exec_signed_load(type) \
  def_EHelper(concat(l,type)){ \
    rtl_lm(s,s0,dsrc1,id_src2->simm,EXPAND_LEN(type));\
    rtl_sext(s,ddest,s0,EXPAND_LEN(type));\
  }

#define def_exec_store(type) \
  def_EHelper(concat(s,type)){ \
    rtl_sm(s,ddest,dsrc1,id_src2->simm,EXPAND_LEN(type));\
  }

#define def_exec_load_store_all(type) \
  def_exec_signed_load(type) \
  def_exec_unsigned_load(type)\
  def_exec_store(type)

#define def_exec_load_store_unsigned(type) \
  def_exec_unsigned_load(type) \
  def_exec_store(type)


#define expand_exec_load_store_d() def_exec_load_store_unsigned(d)

expand_exec_load_store_d()
def_exec_load_store_all(w)
def_exec_load_store_all(h)
def_exec_load_store_all(b)

def_EHelper(lui){
  rtl_li(s, ddest, id_src1->simm);
}
