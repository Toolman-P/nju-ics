#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  Log("[STRACE] Syscall ID: %d",a[0]);
  switch (a[0]) {
    case SYS_EXIT:
      halt(0);
      break;
    case SYS_YIELD:
      yield();
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
