#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int itoa(int a,char *str){
  static char buf[150];
  char *bp = buf,*sp = str;
  int cnt = 0;

  if(a==0){
    *str='0';
    *(str+1)='\0';
    return 1;
  }

  while(a){
    *bp++=a%10+'0';
    a/=10;cnt++;
  }
  bp = buf+cnt;
  while(bp!=buf)
    *sp++=*(--bp);
  return cnt;
}

int printf(const char *fmt, ...) {
  
  static char buf[1000];
  const char *fp=fmt;
  va_list ap;
  char *pt;
  int cnt = 0;

  va_start(ap,fmt);
  while(*fp!='\0'){
    if(*fp=='%'){
      switch(*(++fp)){
        case '%':
          putch('%');
          cnt++;
          break;
        case 'd':
          cnt+=itoa(va_arg(ap,int),buf);
          putstr(buf);
          break;
        case 's':
          pt=va_arg(ap,char *);
          cnt+=strlen(pt);
          putstr(pt);
          break;
        default:
          panic("Not implemented!");
          return -1;
      }
    }else{
      putch(*fp);
      cnt++;
    }
    fp++;
  }
  putch('\0');
  va_end(ap);
  return cnt;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  static char buf[1000];
  char *op=out,*pt;
  const char *fp=fmt;
  int cnt=0;
  while(*fp!='\0'){
    if(*fp=='%'){
      switch(*(++fp)){
        case '%':
          *op++='%';
          cnt++;
          break;
        case 'd':
          cnt+=itoa(va_arg(ap,int),buf);
          for(pt=buf;*pt;op++,pt++)
            *op=*pt;
          break;
        case 's':
          for(pt=va_arg(ap,char *),cnt+=strlen(pt);*pt;pt++,op++)
            *op=*pt;
          break;
        default:
          return -1;
      }
    }else{
      *op++=*fp;
      cnt++;
    }
    fp++;
  }
  *op='\0';
  return cnt;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int done;

  va_start(ap,fmt);
  done = vsprintf(out,fmt,ap);
  va_end(ap);
  return done;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
