#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i;
  for(i=0;s[i]!='\0';i++);
  return i;
}

char *strcpy(char *dst, const char *src) {
  size_t len = strlen(src);
  for(size_t i=0;i<len;i++)
    dst[i]=src[i];
  dst[len]='\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  size_t len = strlen(src);
  for(i=0;i<len&&src[i];i++)
    dst[i]=src[i];
  for(;i<len;i++)
    dst[i]='\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t len = strlen(src);
  size_t i,pos;
  for(i=0,pos=strlen(dst);i<len;i++)
    dst[pos+i]=src[i];
  dst[pos+i]='\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  
  while(*s1&&*s2){
    if(*s1!=*s2)
      return (*s1)-(*s2);
    s1++;s2++;
  }
  if(*s1)
    return (int)(*s1);
  if(*s2)
    return -(int)(*s2);
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while(*s1&&*s2&&n--){
    if(*s1!=*s2)
      return (*s1)-(*s2);
    s1++;s2++;
  }
  if(!n)
    return 0;
  if(*s1)
    return (int)(*s1);
  if(*s2)
    return -(int)(*s2);
  return -1;
}

void *memset(void *s, int c, size_t n) {
  for(size_t i=0;i<n;i++)
    *(char *)(s+i) = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *p1 = dst;
  const char *p2 = src;

  if(IN_RANGE(dst,RANGE(src,dst+n))){
    for(p1+=n,p2+=n;n--;)
      *--p1=*--p2;
  }
  else{
    while(n--)
      *p1++=*p2++;
  }
    

  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for(size_t i=0;i<n;i++)
    *(char *)(out+i) = *(const char *)(in+i);
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *p1 = s1,*p2=s2;
  while(*p1&&*p2&&n--){
    if(*p1!=*p2)
      return (*p1)-(*p2);
    p1++;p2++;
  }
  if(!n)
    return 0;
  if(*p1)
    return (int)(*p1);
  if(*p2)
    return -(int)(*p2);
  return -1;
}

#endif
