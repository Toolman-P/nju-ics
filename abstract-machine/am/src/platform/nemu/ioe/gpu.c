#include <am.h>
#include <nemu.h>


#define SYNC_ADDR (VGACTL_ADDR + 4)

static uint32_t vga_w = 0;
static uint32_t vga_h = 0;

#define PIXEL(i,j,ctl) \
  *(uint32_t *)(ctl->pixels+4*(j*ctl->w+i))

#define VMEM(i,j,ctl) \
  (FB_ADDR+4*(vga_w*(ctl->y+j)+ctl->x+i))

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
  
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }else{
    int w=ctl->w,h=ctl->h;
    if(w==0||h==0)
      return;
    for(int i=0;i<w;i++)
      for(int j=0;j<h;j++){
        outl(VMEM(i,j,ctl),PIXEL(i,j,ctl));
      }  
  } 
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
