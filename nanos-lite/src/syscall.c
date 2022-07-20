#include <common.h>
#include <fs.h>
#include "syscall.h"

struct timeval{
  long tv_sec;
  long tv_usec;
};

#define define_syscall(name) \
  static inline void CONCAT(do_syscall_,name)(Context *c)

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime);

define_syscall(write){
  c->GPRx=write(c->GPR2,(const void *)c->GPR3,c->GPR4);
}

define_syscall(sbrk){
  c->GPRx=0;
}

define_syscall(read){
  c->GPRx=read(c->GPR2,(void *)c->GPR3,c->GPR4);
}

define_syscall(open){
  c->GPRx = open((const char *)c->GPR2);
}

define_syscall(close){
  c->GPRx = close(c->GPR2);
}

define_syscall(lseek){
  c->GPRx = lseek(c->GPR2,c->GPR3,c->GPR4); 
}

define_syscall(gettimeofday){
  AM_TIMER_UPTIME_T uptime;
__am_timer_uptime(&uptime);
  ((struct timeval *)c->GPR2)->tv_sec = (uptime.us / 1000000);
  ((struct timeval *)c->GPR2)->tv_usec = (uptime.us % 1000000);
  c->GPRx = 0;
}

void do_syscall(Context *c) {
#ifdef STRACE
  Log("[STRACE] Syscall ID: %d\n",c->GPR1);
#endif
  switch (c->GPR1) {
    case SYS_exit:
      halt(c->GPR2);
      break;
    case SYS_yield:
      yield();
      break;
    case SYS_read:
      do_syscall_read(c);
      break;
    case SYS_write:
      do_syscall_write(c);
      break;
    case SYS_brk:
      do_syscall_sbrk(c);
      break;
    case SYS_open:
      do_syscall_open(c);
      break;
    case SYS_close:
      do_syscall_close(c);
      break;
    case SYS_lseek:
      do_syscall_lseek(c);
      break;
    case SYS_gettimeofday:
      do_syscall_gettimeofday(c);
      break;
    default: panic("Unhandled syscall ID = %d", c->GPR1);
  }
}
