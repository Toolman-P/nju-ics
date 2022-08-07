#include <am.h>
#include <nemu.h>
#include <klib.h>

#define CONFIG_MAP_TRACE 0

#define MAX_LEVELS 2
#define PPN_MASK 0xFFFFFFFFFFFUL
#define PTETOPA(pte) ((PTE *)(((pte>>10UL)&PPN_MASK)<<12UL)) 
#define PATOPTE(pa) (((((PTE)pa)>>12UL)&PPN_MASK)<<10UL)
#define PTE_DEFAULT (PTE_X|PTE_W|PTE_R|PTE_V)
#define PPN_PART_MASK 0x1FFUL

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
  
  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    for (void *va = segments[i].start; va < segments[i].end; va += PGSIZE){
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;
  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {

  PTE *ptr,*next = NULL;
  PTE pte;

  uintptr_t vpn[] = {
      ((uintptr_t)va>>12UL)&PPN_PART_MASK,
      ((uintptr_t)va>>21UL)&PPN_PART_MASK,
      ((uintptr_t)va>>30UL)&PPN_PART_MASK
  };
  int i;
  #if CONFIG_MAP_TRACE
    printf("as.ptr:%p\tva:%p\n",as->ptr,va);
  #endif
  assert(as->ptr!=0);

  for(i=2,ptr = (PTE *)(as->ptr);i>0;i--){
    ptr += vpn[i];
    pte = *ptr;
    

    if(pte & PTE_V){
      next = PTETOPA(pte);
    }else{
      next = pgalloc_usr(PGSIZE);
      *ptr = PATOPTE(next) | PTE_V;
    }

    ptr = next;
  }

  ptr += vpn[i];

  assert((*ptr & PTE_V) == 0);
  *ptr = PATOPTE(pa) | PTE_DEFAULT;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *cp = (Context *)(kstack.end - sizeof(Context)); 
  assert(cp != 0);
  cp->mepc = (uintptr_t)entry;
  cp->pdir = as->ptr;
  return cp;
}
