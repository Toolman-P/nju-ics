#include <am.h>
#include <nemu.h>


#define SYNC_ADDR (VGACTL_ADDR + 4)

static uint32_t vga_w = 0;
static uint32_t vga_h = 0;

void __am_gpu_init() {
  
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  
  uint32_t info = inl(VGACTL_ADDR);
  vga_w = info>>16;
  vga_h = info&0xFFFF;

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = vga_w, .height = vga_h,
    .vmemsz = vga_w*vga_h*sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {

  int w=ctl->w,h=ctl->h,x=ctl->x,y=ctl->y;
  
  uint32_t *fb = (uint32_t *)FB_ADDR;
  uint32_t *pixels = (uint32_t *)ctl->pixels;
  for(int i=0;i<w;i++)
    for(int j=0;j<h;j++)
      fb[(y+j)*vga_w+x+i]=pixels[j*w+i];  

  if (ctl->sync)
    outl(SYNC_ADDR, 1);
     
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
