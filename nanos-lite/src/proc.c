#include <proc.h>
#include <memory.h>

#define MAX_NR_PROC 4

#define STACK_START(pcb) ((void *)pcb->stack)
#define STACK_END(pcb) ((void *)(pcb->stack+PGSIZE-1))
#define AREA_STACK(pcb) ((Area){STACK_START(pcb),STACK_END(pcb)})

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {}; 
PCB *current = NULL, *next = NULL;

Context* kcontext(Area kstack, void (*entry)(void *), void *arg);
Context* ucontext(AddrSpace *as, Area kstack, void *entry);

void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
int context_uload(PCB *pcb, const char *filename, char * const argv[], char * const envp[]);

uintptr_t loader(PCB *pcb, const char *filename);

void naive_uload(PCB *pcb, const char *filename);

void context_kload(PCB *pcb,void (*entry)(void *), void *arg){
  pcb->as.area = AREA_STACK(pcb);
  pcb->cp = kcontext(AREA_STACK(pcb),entry,arg); 
}

int context_uload(PCB *pcb, const char *filename, char * const argv[], char * const envp[]){
  
  protect(&pcb->as);
  
  char * const *sp;
  void *start, *entry, *va, *pa;

  intptr_t argc = 0;
  
  pa = pg_alloc(8*PGSIZE);

  Area pa_area = (Area){pa+8*PGSIZE,pa+8*PGSIZE};
  Area va_area = (Area){pcb->as.area.end-8*PGSIZE,pcb->as.area.end};
  
  for(va=va_area.start;va<va_area.end;va+=PGSIZE,pa+=PGSIZE)
      map(&pcb->as,va,pa,7);
  
  for(sp=argv;sp&&*sp;sp++){
    pa_area.start -= (strlen(*sp)+1+sizeof(uintptr_t));
    argc++;
  }

  pa_area.start -= sizeof(uintptr_t); //NULL

  for(sp=envp;sp&&*sp;sp++)
    pa_area.start -= (strlen(*sp)+1+sizeof(uintptr_t)); //NULL

  pa_area.start -= sizeof(uintptr_t);
  pa_area.start -= sizeof(intptr_t);
  
  start = va_area.end - (pa_area.end - pa_area.start); // the stack pointer 

  *(intptr_t *)(pa_area.start) = argc;
  pa_area.start = (void *)(((intptr_t *)pa_area.start) + 1); // 准备用户栈


  for(sp=argv;sp&&*sp;sp++){
    pa_area.end -= (strlen(*sp)+1);
    memcpy(pa_area.end,*sp,strlen(*sp)+1);
    
    *(char **)(pa_area.start) = (char *)pa_area.end;
    pa_area.start = (void *)(((char **)pa_area.start) + 1);
  }

  *(uintptr_t **)(pa_area.start) = (uintptr_t *)NULL;
  pa_area.start = (void *)(((char **)pa_area.start) + 1);

  for(sp=envp;sp&&*sp;sp++){
    pa_area.end -= (strlen(*sp)+1);
    memcpy(pa_area.end,*sp,strlen(*sp)+1);
    *(char **)(pa_area.start) = (char *)pa_area.end;
    pa_area.start = (void *)(((char **)pa_area.start) + 1);
  }

  *(uintptr_t **)(pa_area.start) = (uintptr_t *)NULL;
  
  if((entry = (void *)loader(pcb,filename)) == NULL)
    return 0;

  Log("entry: %p",entry);
  Log("sp: %p",start);

  pcb->cp = ucontext(&pcb->as,AREA_STACK(pcb),entry); 
  pcb->cp->GPRx = (uintptr_t)start;// switch to the context pointer
  return 1;
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

PCB *fetch_available_pcb(){
  for(int i=0;i<MAX_NR_PROC;i++)
    if(current != &pcb[i] && pcb[i].as.area.start == 0){
      next = &pcb[i];
      return next;
    }
  panic("No more free process to allocate.");
  return NULL;
}

void halt_current_process(){
  memset(current,0,sizeof(PCB));
}

PCB *poll_running_pcb(){
  for(int i=0;i<MAX_NR_PROC;i++)
    if(current != &pcb[i] && pcb[i].as.area.start != 0){
      next = &pcb[i];
      return next;
    }
  panic("No running process left");
}

void init_proc() {
  Log("Initializing processes...");

  // static int i = 114514;
  // context_kload(&pcb[0],hello_fun,&i);

  static char * const argv[]={
    "/bin/hello",
    NULL
  };

  static char * const envp[]={
    "PATH = /bin:/usr/bin",
    NULL
  };
  context_uload(&pcb[0],argv[0],argv,envp);
  context_uload(&pcb[1],argv[0],argv,envp);
  switch_boot_pcb();
}

Context *schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0]) ? &pcb[1] : &pcb[0];
  return current->cp;
}
