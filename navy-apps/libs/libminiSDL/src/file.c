#include <sdl-file.h>
#include <assert.h>
#include <memory.h>
#define MAX_OPS 8
#define min(a,b) ((a)<(b)?(a):(b))

static SDL_RWops ops[MAX_OPS];

static inline int64_t __SDL_RW_size(struct SDL_RWops *f){
  switch(f->type){
    case RW_TYPE_FILE:
      int64_t ret,pos;
      pos = ftell(f->fp);
      fseek(f->fp,0,SEEK_END);
      ret = ftell(f->fp);
      fseek(f->fp,pos,SEEK_SET);
      return ret;
    case RW_TYPE_MEM:
      return f->mem.size;
    default:
      assert(0);
  }
}

static inline int64_t __SDL_RW_seek (struct SDL_RWops *f, int64_t offset, int whence){
  switch(f->type){
    case RW_TYPE_FILE:
      return fseek(f->fp,offset,whence);
    case RW_TYPE_MEM:
      return offset;
    default:
      assert(0);
  }
}

static inline size_t __SDL_RW_read (struct SDL_RWops *f, void *buf, size_t size, size_t nmemb){
  switch(f->type){
    case RW_TYPE_FILE:
      return fread(buf,size,nmemb,f->fp);
    case RW_TYPE_MEM:
      size_t sz = min((size*nmemb),f->mem.size);
      memcpy(buf,f->mem.base,sz);
      return sz;
    default:
      assert(0);
  }
}

static inline size_t __SDL_RW_write(struct SDL_RWops *f, const void *buf, size_t size, size_t nmemb){
  switch(f->type){
    case RW_TYPE_FILE:
      return fwrite(buf,size,nmemb,f->fp);
    case RW_TYPE_MEM:
      size_t sz = min((size*nmemb),f->mem.size);
      memcpy(f->mem.base,buf,sz);
      return sz;
    default:
      assert(0);
  }
}

static inline int __SDL_RW_close(struct SDL_RWops *f){
    f->type = -1;
    switch(f->type){
    case RW_TYPE_FILE:
      return fclose(f->fp);
    case RW_TYPE_MEM:
      f->mem.base=NULL;
      return 0;
    default:
      assert(0);
  }
}

void SDL_init_RWops(){
  for(int i=0;i<MAX_OPS;i++){
    ops[i].read = __SDL_RW_read;
    ops[i].write = __SDL_RW_write;
    ops[i].size = __SDL_RW_size;
    ops[i].close = __SDL_RW_close;
    ops[i].seek = __SDL_RW_seek;
    ops[i].type = -1;
  }
}

SDL_RWops* SDL_RWFromFile(const char *filename, const char *mode) {
  for(int i=0;i<MAX_OPS;i++)
    if(ops[i].type == -1){
      ops[i].fp = fopen(filename,mode);
      ops[i].type = RW_TYPE_FILE;
      return &ops[i];
    }
  return NULL;
}

SDL_RWops* SDL_RWFromMem(void *mem, int size) {
  for(int i=0;i<MAX_OPS;i++)
    if(ops[i].type == -1){
      ops[i].mem.base = mem;
      ops[i].mem.size = size;
      return &ops[i];
    }
  return NULL;
}
