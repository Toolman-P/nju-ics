#include <proc.h>

#define MAX_NR_PROC 4

#define STACK_START(pcb) ((void *)pcb->stack)
#define STACK_END(pcb) ((void *)(pcb->stack+PGSIZE-1))
#define AREA_STACK(pcb) ((Area){STACK_START(pcb),STACK_END(pcb)})

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

Context* kcontext(Area kstack, void (*entry)(void *), void *arg);
Context* ucontext(AddrSpace *as, Area kstack, void *entry);

void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename, char * const argv[], char * const envp[]);
uintptr_t loader(PCB *pcb, const char *filename);
void naive_uload(PCB *pcb, const char *filename);

void context_kload(PCB *pcb,void (*entry)(void *), void *arg){
  pcb->cp = kcontext(AREA_STACK(pcb),entry,arg); 
}

void context_uload(PCB *pcb, const char *filename, char * const argv[], char * const envp[]){

  Context *cp = ucontext(&pcb->as,AREA_STACK(pcb),(void *)loader(pcb,filename));
  Area area = (Area){(void *)cp->GPRx,(void *)cp->GPRx};

  char * const *sp;
  void * start;
  intptr_t argc = 0;

  for(sp=argv;*sp;sp++){
    area.start -= (strlen(*sp)+1+sizeof(uintptr_t));
    argc++;
  }

  area.start -= sizeof(uintptr_t); //NULL

  for(sp=envp;*sp;sp++)
    area.start -= (strlen(*sp)+1+sizeof(uintptr_t)); //NULL

  area.start -= sizeof(uintptr_t);
  area.start -= sizeof(intptr_t);
  start = area.start;

  *(intptr_t *)(area.start) = argc;
  area.start = (void *)(((intptr_t *)area.start) + 1);

  for(sp=argv;*sp;sp++){
    area.end -= (strlen(*sp)+1);
    memcpy(area.end,*sp,strlen(*sp)+1);
    
    *(char **)(area.start) = (char *)area.end;
    area.start = (void *)(((char **)area.start) + 1);
  }

  *(uintptr_t **)(area.start) = (uintptr_t *)NULL;
  area.start = (void *)(((char **)area.start) + 1);

  for(sp=envp;*sp;sp++){
    area.end -= (strlen(*sp)+1);
    memcpy(area.end,*sp,strlen(*sp)+1);
    *(char **)(area.start) = (char *)area.end;
    area.start = (void *)(((char **)area.start) + 1);
  }

  *(uintptr_t **)(area.start) = (uintptr_t *)NULL;

  cp->GPRx = (uintptr_t)start;
  pcb->cp = cp;
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with %d for the %dth time!",*(int *)arg,j);
    j ++;
    yield();
  }
}

void init_proc() {
  Log("Initializing processes...");

  char * const argv[] = {
    "argv is here",
    "something",
    "maybe",
    NULL
  };

  char * const envp[] = {
    "PATH = /bin",
    "something new",
    NULL
  };
  Log("Initializing uload...");
  context_uload(&pcb[0],"/bin/args-test",argv,envp);
  Log("Finishing Initializing uload...");
  switch_boot_pcb();
}

Context *schedule(Context *prev) {
  current->cp = prev;
  current = &pcb[0];
  return current->cp;
}
