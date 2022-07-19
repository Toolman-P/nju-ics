#include <common.h>
#include <fs.h>
#include "syscall.h"

#define define_syscall(name) \
  static inline void CONCAT(do_syscall_,name)(Context *c)

define_syscall(write){
  int fd = c->GPR2;
  void *buf = (void *)c->GPR3;
  int len = c->GPR4;
  switch(fd){
    case 0:
      panic("STDIN is not allowed as fd");
      break;
    case 1:
    case 2:
      c->GPRx=len;
      while(len--)
        putch(*(char *)buf++);
      break;
    default:
      c->GPRx=fs_write(fd,buf,len);
  }
}

define_syscall(sbrk){
  c->GPRx=0;
}

define_syscall(read){
  int fd = c->GPR2;
  void *buf = (void *)c->GPR3;
  int len = c->GPR4;
  switch(fd){
    case 0:
    case 1:
    case 2:
      panic("Sorry pipe is not implemented so far.");
      break;
    default:
      c->GPRx=fs_read(fd,buf,len);
  }
}

define_syscall(open){
  const char *filename = (const char *)c->GPR2;
  c->GPRx = fs_open(filename);
}

define_syscall(close){
  c->GPRx = fs_close(c->GPR2);
}

define_syscall(lseek){
  c->GPRx = fs_lseek(c->GPR2,c->GPR3,c->GPR4); 
}

void do_syscall(Context *c) {
#ifdef STRACE
  Log("[STRACE] Syscall ID: %d\nz",a[0]);
#endif
  switch (c->GPR1) {
    case SYS_exit:
      halt(0);
      break;
    case SYS_yield:
      yield();
      break;
    case SYS_read:
      break;
    case SYS_write:
      do_syscall_write(c);
      break;
    case SYS_brk:
      do_syscall_sbrk(c);
      break;
    case SYS_open:
      do_syscall_open(c);
    case SYS_close:
      do_syscall_close(c);
      break;
    case SYS_lseek:
      do_syscall_lseek(c);
      break;
    default: panic("Unhandled syscall ID = %d", c->GPR1);
  }
}
