#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);
Context* __am_irq_handle(Context *c) {
  if (user_handler) {
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
      default:
        ev.event = EVENT_ERROR;
        break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);

    if(c->pdir!=NULL)
      __am_switch(c);
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
  cp->pdir = NULL;
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
