#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) { 

  /*
     c here is the end of stack
  */

  uintptr_t ksp;

  if (user_handler) {
    
    asm volatile("csrr %0, mscratch" : "=r"(ksp)); // read ksp here
    
    c->np = (ksp == 0) ? KERNEL_MODE : USER_MODE;  

    ksp = 0; 

    asm volatile("csrw mscratch,%0" :: "r"(ksp)); // rewrite ksp (set ksp to 0) in case for CTE reentry
    
    if(c->pdir!=NULL)
      __am_get_cur_as(c);
    
    Event ev = {0};
    switch (c->mcause) {
      case MCAUSE_ECALL:
        switch(c->GPR1){
          case INTR_YIELD:
            ev.event = EVENT_YIELD;
            break;
          default:
            ev.event = EVENT_SYSCALL;
            break;
        }
        break;
      case MCAUSE_TIMER:
        ev.event = EVENT_IRQ_TIMER;
        break;
      default:
        ev.event = EVENT_ERROR;
        break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);

    if(c->pdir!=NULL)
      __am_switch(c); // c the end of kernel stack
    
    if(c->np == USER_MODE){
      ksp = ((uintptr_t)c)+sizeof(Context); // let ksp be the new start of kernel stack
      asm volatile("csrw mscratch,%0" :: "r"(ksp));
    }
  }
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;
  
  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  
  Context *cp = (Context *)(kstack.end - sizeof(Context));
  cp->mepc = (uintptr_t)entry;
  cp->a0 = (uintptr_t)arg;
  cp->gpr[2] = (uintptr_t)cp;
  
  cp->pdir = NULL;
  cp->mstatus = (1ul<<7ul) | (1ul<<3ul);
  cp->np = KERNEL_MODE;
  return cp;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
  
}
