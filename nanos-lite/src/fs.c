#include <fs.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define INBOUND(a,sz) (min(max(0,a),sz))

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);


size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t __gpu_screen_size();

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_DISPLAY, FD_EVENTS};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write, 0},
  [FD_DISPLAY] = {"/proc/dispinfo",0, 0, dispinfo_read, invalid_write, 0},
  [FD_EVENTS] = {"/dev/events",0 , 0, events_read, invalid_write, 0},
#include "files.h"
};

static size_t file_total = 0;

int open(const char *filename){
  for(int i=0;i<file_total;i++)
    if(strcmp(filename,file_table[i].name)==0)
      return i;
  return -1;
}


size_t fs_read(int fd, void *buf, size_t len){
  Finfo *fi = &file_table[fd];
  size_t offset = fi->disk_offset + fi->open_offset;
  size_t rdsz = min(len,fi->size-fi->open_offset);
  fi->open_offset = fi->open_offset+rdsz;
  return ramdisk_read(buf,offset,rdsz);
}

size_t fs_write(int fd, const void *buf, size_t len){
  Finfo *fi = &file_table[fd];
  size_t offset = fi->disk_offset + fi->open_offset;
  size_t wtsz = min(len,fi->size-fi->open_offset);
  fi->open_offset = fi->open_offset+wtsz;
  return ramdisk_write(buf, offset, wtsz);
}

size_t lseek(int fd, size_t offset, int whence){
  Finfo *fi = &file_table[fd];
  switch(whence){
    case SEEK_SET:
     fi->open_offset = INBOUND(offset,fi->size);
     break; 
    case SEEK_CUR:
     fi->open_offset = INBOUND(offset+fi->open_offset,fi->size);
     break;
    case SEEK_END:
     fi->open_offset = INBOUND(fi->size+offset,fi->size);
     break;
    default:
      assert(0);
  }
  return fi->open_offset;
}

size_t close(int fd){
  file_table[fd].open_offset = 0;
  return 0;
}

size_t read(int fd, void *buf, size_t len){
  ReadFn rfn = file_table[fd].read;
  if(rfn)
    return rfn(buf,file_table[fd].open_offset,len);
  else
    return fs_read(fd,buf,len);
}

size_t write(int fd, const void *buf, size_t len){
  WriteFn wfn = file_table[fd].write;
  if(wfn){
    return wfn(buf,file_table[fd].open_offset,len);
  }
  else
    return fs_write(fd,buf,len);
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_total = sizeof(file_table)/sizeof(Finfo)-1;
  file_table[FD_FB].size = __gpu_screen_size();
}
