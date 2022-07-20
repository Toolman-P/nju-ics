#include <stdio.h>
#include <string.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  while (1) {
    char buf[64];
    if (NDL_PollEvent(buf, sizeof(buf))) {
      if(strcmp(buf,"NONE")!=0)
        printf("receive event: %s\n", buf);
    }
  }
  return 0;
}
