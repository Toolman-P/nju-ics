#include <memory.h>
#include <proc.h>

static void *pf = NULL;

extern void *elf_page;

void* new_page(size_t nr_page) {
  void *ret = pf;
  pf += nr_page * PGSIZE;
  return ret;
}

#ifdef HAS_VME
void* pg_alloc(int n) {
  assert(n%PGSIZE == 0);
  void *ptr = new_page(n/PGSIZE);
  memset(ptr,0,n);
  return ptr;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  void *pa,*va;
  Area va_area;

  if(current->max_brk==0){
    if((void *)ROUNDDOWN(brk,PGSIZE)>elf_page){
      pa = pg_alloc(PGSIZE);
      map(&current->as,(void *)ROUNDDOWN(brk,PGSIZE),pa,7);
    }
    current->max_brk = brk;
  }else{
    if(brk>current->max_brk){
      
      if((void *)brk>current->as.area.end)
        return -1;
    
      if(ROUNDDOWN(brk,PGSIZE)+PGSIZE>ROUNDUP(current->max_brk,PGSIZE)){
        va_area = (Area){(void *)ROUNDUP(current->max_brk,PGSIZE),
                        (void *)ROUNDDOWN(brk,PGSIZE)+PGSIZE};
        pa = pg_alloc(va_area.end - va_area.start);
        for(va=va_area.start;va<va_area.end;va+=PGSIZE,pa+=PGSIZE)
          map(&current->as,va,pa,7); // allocate new page in the current address space
      }
      current->max_brk = brk;
    }
  }
  
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
  Log("VME init complete.");
#endif
}
