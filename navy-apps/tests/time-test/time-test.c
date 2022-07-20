#include <stdio.h>
#include <NDL.h>
#include <assert.h>

int main ()
{
  int i=1;
  while(1){
    int ms = NDL_GetTicks();
    if(ms > i*500){
      printf("Hello from time-test with 0.5s interval.\n");
      i++;
    }
  }  
  return 0;
}
