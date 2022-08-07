#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <syscall.h>
#include <fcntl.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  uint32_t usec = tv.tv_usec;
  uint32_t sec = tv.tv_sec;
  return (sec*1000)+(usec/1000);
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events",0,0);
  return read(fd,buf,len);
}

void NDL_OpenCanvas(int *w, int *h) {
  char buf[64];
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }else{
    int fd = open("/proc/dispinfo",0,0);
    read(fd,buf,sizeof(buf));
    sscanf(buf,"%d %d",&screen_w,&screen_h);
    *w = screen_w;
    *h = screen_h;
    close(fd);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  size_t p_offset,s_offset;
  int fd = open("/dev/fb",0,0);
  for(int j=0;j<h;j++){
    s_offset = (y+j)*screen_w + x;
    p_offset = w*j;
    lseek(fd,s_offset,SEEK_SET);
    write(fd,pixels+p_offset,w);
  }
  
  if(w==0 && h==0)
    write(fd,NULL,0);
  close(fd);  
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
