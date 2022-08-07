#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

#include "../local-include/reg.h"

#define CONFIG_MMU_TRACE 0

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80

#define PPN_MASK 0xFFFFFFFFFFFUL
#define OFFSET_MASK 0xFFFUL
#define PPN_PART_MASK 0x1FFUL

#define PPN_BASE ((*satp & PPN_MASK)<<PAGE_SHIFT)
#define PTETOPPN(pte) (((pte>>10)&PPN_MASK)<<12) 

#define PTETOPADDR(pte,offset) (PTETOPPN(pte)|offset) 


int isa_mmu_check(vaddr_t vaddr, int len, int type){
  if((*satp>>63) == 0)
    return MMU_DIRECT;
  else
    return MMU_TRANSLATE;
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {

  paddr_t ptr;

  uint64_t pte;
  uint64_t offset = vaddr & OFFSET_MASK;
  
  uint64_t vpn[] = {
    ((uint64_t)vaddr>>12UL)&PPN_PART_MASK,
    ((uint64_t)vaddr>>21UL)&PPN_PART_MASK,
    ((uint64_t)vaddr>>30UL)&PPN_PART_MASK
  };
  int i;

  for(i=2,ptr=PPN_BASE;i>=0;i--){
    
    ptr = ptr + vpn[i] * sizeof(uint64_t);
    pte = paddr_read(ptr,sizeof(uint64_t));

    #if CONFIG_MMU_TRACE 
      printf("vaddr:%p  as.ptr:%p\n",(void *)((uint64_t)vaddr),(void *)((uint64_t)PPN_BASE));
    #endif
    
    if(!(pte & PTE_V))
      panic("something happened at translating pte (%p) with vaddr: %p with pc: %p",
      (void *)pte,(void *)((uint64_t)vaddr),(void *)cpu.pc);
    
    if((pte & PTE_R) | (pte & PTE_W) | (pte & PTE_X))
      return (paddr_t)PTETOPADDR(pte,offset);
    else
      ptr = PTETOPPN(pte);
  }

  assert(0);
  return 0;
}
