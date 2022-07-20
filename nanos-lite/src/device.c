#include <common.h>

#define min(a,b) ((a)<(b)?(a):(b))
#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd);
void __am_gpu_config(AM_GPU_CONFIG_T *cfg);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl);

static AM_GPU_CONFIG_T cfg;

size_t serial_write(const void *buf, size_t offset, size_t len) {
  while(len--)
    putch(*(char *)buf++);
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T kbd;  
  const char *key;
  size_t sz;

  __am_input_keybrd(&kbd);
  key = keyname[kbd.keycode];
  sz=min(len,strlen(key));
  memset(buf,0,len);
  memcpy(buf,key,sz);
  return sz;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  __am_gpu_config(&cfg);
  memset(buf,0,len);
  sprintf((char *)buf,"%d %d\n",cfg.width,cfg.height);
  return strlen(buf); 
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_FBDRAW_T ctl;
  ctl.pixels = (void *)buf;
  ctl.x = offset % cfg.width;
  ctl.y = offset / cfg.width;
  ctl.w = len;
  ctl.h = 1;
  ctl.sync = true;
  __am_gpu_fbdraw(&ctl);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
