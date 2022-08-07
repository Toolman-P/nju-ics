#include <NDL.h>
uint32_t boot_time;

void SDL_init_RWops();
// void SDL_init_Events();

int SDL_Init(uint32_t flags) {
  SDL_init_RWops();
  // SDL_init_Events();
  return NDL_Init(flags);
}

void SDL_Quit() {
  NDL_Quit();
}

char *SDL_GetError() {
  return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
  return -1;
}

int SDL_ShowCursor(int toggle) {
  return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
}
