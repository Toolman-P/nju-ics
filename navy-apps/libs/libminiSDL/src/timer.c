#include <NDL.h>
#include <sdl-timer.h>
#include <malloc.h>

#define MAX_TIMERS 8
static SDL_TimerID timers[MAX_TIMERS] = {NULL};

extern uint32_t boot_time;

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  uint32_t cur = NDL_GetTicks();
  while(NDL_GetTicks()-cur<interval);
  callback(interval,param);
  return &timers[0];
}

int SDL_RemoveTimer(SDL_TimerID id) {
  return 1;
}

uint32_t SDL_GetTicks() {
  return NDL_GetTicks()-boot_time;
}

void SDL_Delay(uint32_t ms) {
  uint32_t cur = NDL_GetTicks();
  while(NDL_GetTicks()-cur<ms);
}

