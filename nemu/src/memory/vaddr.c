#include <isa.h>
#include <memory/paddr.h>

int isa_mmu_check(vaddr_t vaddr, int len, int type);
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type);

#define do_translate_read(vaddr,len,type) \
  do{\
    switch(isa_mmu_check(vaddr,len,type)){ \
      case MMU_DIRECT:\
        return paddr_read(vaddr,len);\
      case MMU_TRANSLATE:\
        return paddr_read(isa_mmu_translate(vaddr,len,type),len);\
      default:\
        assert(0);\
    }\
  }while(0)\

#define do_translate_write(vaddr,len,data,type) \
  do{\
    switch(isa_mmu_check(vaddr,len,type)){ \
      case MMU_DIRECT:\
        paddr_write(vaddr,len,data);\
        break;\
      case MMU_TRANSLATE:\
        paddr_write(isa_mmu_translate(vaddr,len,type),len,data);\
        break;\
      default:\
        assert(0);\
    }\
  }while(0)\

word_t vaddr_ifetch(vaddr_t addr, int len) {
  do_translate_read(addr,len,MEM_TYPE_IFETCH);
  return 0;
}

word_t vaddr_read(vaddr_t addr, int len) {
  do_translate_read(addr,len,MEM_TYPE_READ);
  return 0;
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  do_translate_write(addr,len,data,MEM_TYPE_WRITE);
}
