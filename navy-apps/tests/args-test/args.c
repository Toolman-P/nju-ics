#include <stdio.h>

int main(int argc, char *argv[], char *envp[]){
    printf("total args:%d\n",argc);
    while(*argv)
        printf("%s\n",*argv++);
    while(*envp)
        printf("%s\n",*envp++);
    while(1);
    return 0;
}