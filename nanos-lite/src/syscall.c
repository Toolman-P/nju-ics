#include <common.h>
#include "syscall.h"

#define define_syscall(name) \
  static inline void CONCAT(do_syscall_,name)(Context *c)

define_syscall(write){
  int fd = c->GPR2;
  char *buf = (char *)c->GPR3;
  int len = c->GPR4;
  switch(fd){
    case 1:
    case 2:
      c->GPRx=len;
      while(len--)
        putch(*buf++);
      break;
    default:
      panic("Wrong fd ID: %d, Only support stdout(1),strerr(2)",fd);
  }
}

define_syscall(sbrk){
  c->GPRx=0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  Log("[STRACE] Syscall ID: %d\n",a[0]);
  switch (a[0]) {
    case SYS_exit:
      halt(0);
      break;
    case SYS_yield:
      yield();
      break;
    case SYS_write:
      do_syscall_write(c);
      break;
    case SYS_brk:
      do_syscall_sbrk(c);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
