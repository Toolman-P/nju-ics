#include <NDL.h>
#include <SDL.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define keyname(k) #k,
typedef struct __SDL_EVENTLIST{
  SDL_Event ev;
  struct __SDL_EVENTLIST *next;
}SDL_EventList_t;

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static SDL_EventList_t *eq_front= NULL;
static SDL_EventList_t *eq_rear = NULL;

static inline int __SDL_eqpush(SDL_Event *ev){
  if(!eq_rear){
    eq_front = eq_rear = malloc(sizeof(SDL_EventList_t));
  }else{
    eq_rear->next = malloc(sizeof(SDL_EventList_t));
    eq_rear = eq_rear->next;
    eq_rear->ev = *ev;
  }
  return 1;
}

static inline int __SDL_eqpop(SDL_Event *ev){
  
  if(!eq_front)
    return 0;
  
  SDL_EventList_t *p;

  if(eq_front == eq_rear){
    *ev = eq_front->ev;
    p = eq_front;
    eq_front = eq_rear = NULL;
  }else{
    p = eq_front;
    eq_front = eq_front->next;
    *ev = p->ev;
  }

  free(p);

  return 1;
}

static inline int __SDL_eqempty(){
  return eq_front == NULL;
}

static inline int __SDL_eqpeek(SDL_Event *events, int numevents, uint32_t mask){
  
  int ret = 0;

  for(SDL_EventList_t *p = eq_front;p!=NULL&&numevents!=0;p=p->next,numevents--){
    if(SDL_EVENTMASK(p->ev.type) & mask){
      *events++=p->ev;
      ret++;
    }
  }

  return ret;
}

static inline int __SDL_eqget(SDL_Event *events, int numevents, uint32_t mask){
  int ret = 0;
  SDL_EventList_t *p = eq_front, *q = NULL, *choice;

  while(p != NULL && numevents--){
    if(SDL_EVENTMASK(p->ev.type) & mask){
      if(!q){
        choice = p;
        eq_front = p->next;
      }else{
        choice = p;
        q->next = p->next;
      }
      ret++;
      *events++ = choice->ev;
      free(choice); 
    }
    q = p;
    p = p->next;
  }

  return ret;
}

static inline int __SDL_fetchevent(SDL_Event *ev){
  static char buf[32];
  char *type,*key;
  NDL_PollEvent(buf,32);

  type = strtok(buf,"_");
  key  = strtok(NULL,"_");
  
  for(int i=0;i<(sizeof(keyname)/sizeof(const char *));i++)
    if(strcmp(key,keyname[i])==0){
      ev->key.keysym.sym = i;
      if(strcmp(type,"UP")==0){
        ev->type = SDL_KEYUP;
        break;
      }
      if(strcmp(type,"DOWN")==0){
        ev->type = SDL_KEYDOWN;
        break;
      }
    }
  
  return ev->key.keysym.sym;
}

int SDL_PushEvent(SDL_Event *ev) {
  __SDL_eqpush(ev);
  return 1;
}

int SDL_PollEvent(SDL_Event *ev) {
  if(!__SDL_eqempty()){
    __SDL_eqpop(ev);
    return 1;
  }else{
    return 0;
  }
}

int SDL_WaitEvent(SDL_Event *event) {

  if(!__SDL_eqempty()){
    __SDL_eqpop(event);
    return 1;
  }else{
    while(__SDL_fetchevent(event)==0);
    return 1;
  }
  assert(0);
  return -1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  switch(action){
    case SDL_ADDEVENT:
      for(int i=0;i<numevents;i++)
        __SDL_eqpush(ev++);
      return numevents;
    case SDL_PEEKEVENT:
      return __SDL_eqpeek(ev,numevents,mask);
    case SDL_GETEVENT:
      return __SDL_eqget(ev,numevents,mask);
  }
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  static uint8_t keystates[128];
  SDL_Event event;

  memset(keystates,0,sizeof(keystates));
  
  if(__SDL_fetchevent(&event))
    keystates[event.key.keysym.sym] = 1;
  return keystates;
}
