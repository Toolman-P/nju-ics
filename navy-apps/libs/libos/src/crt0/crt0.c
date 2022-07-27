#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  char *empty[] =  {NULL };
  int argc;
  char **argv,**envp;

  argc = *(intptr_t *)args;
  argv = (char **)(++args);
  envp = args+argc+1;
  environ = envp;

  exit(main(argc,argv,envp));
  assert(0);
}
