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

static AM_GPU_CONFIG_T __gpu_cfg;

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

  if(kbd.keydown)
    strcpy(buf,"DOWN_");
  else
    strcpy(buf,"UP_");
  
  assert(len>strlen(key));
  strcat(buf,key);

  return sz;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  memset(buf,0,len);
  sprintf((char *)buf,"%d %d\n",__gpu_cfg.width,__gpu_cfg.height);
  return strlen(buf); 
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_FBDRAW_T ctl;
  if(len){
    ctl.pixels = (void *)buf;
    ctl.x = offset % __gpu_cfg.width;
    ctl.y = offset / __gpu_cfg.width;
    ctl.w = len;
    ctl.h = 1;
    ctl.sync = false;
  }else{
    ctl.pixels = NULL;
    ctl.x = ctl.y = ctl.w = ctl.h = 0;
    ctl.sync = true;
  }
  __am_gpu_fbdraw(&ctl);
  return len;
}

static void init_gpu(){
  __am_gpu_config(&__gpu_cfg);
}

size_t __gpu_screen_size(){
  return __gpu_cfg.width * __gpu_cfg.height;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
  init_gpu();
}
