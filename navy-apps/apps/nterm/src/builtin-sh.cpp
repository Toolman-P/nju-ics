#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static int cmd_echo(char *args){
  if(!args)
    return 0;
  char *str = strtok(NULL," ");
  sh_printf("%s\n",str);
  return 1;
}

static int cmd_exit(char *args){
  exit(0);
  return -1;
}

static struct{
  const char *cmd;
  int (*handler)(char *);
}cmd_table[] = {
  {"echo",cmd_echo},
  {"exit",cmd_exit}
};
#define ARR_LEN(arr) (sizeof(arr)/sizeof(arr[0]))
#define NR_CMD ARR_LEN(cmd_table)

static void copy_args(char *args,const char *cmd){
  char *p = args;
  const char *q = cmd;
  while(*q != '\n')
    *p++=*q++;
}

static void sh_handle_cmd(const char *cmd) {
  char args[256] = {'\0'};
  char *argv[256] = {NULL};
  char **ap = argv;

  copy_args(args,cmd);
  
  if(*args == '\0')
    return;
  
  char *arg = strtok(args," ");

  if(arg[0] == '\0' || arg==NULL)
    return;
  
  for(int i=0;i<NR_CMD;i++)
    if(strcmp(arg,cmd_table[i].cmd)==0){
      cmd_table[i].handler(arg + strlen(arg) + 1); //maybe need further parsing
      return;
    }
  printf("no command found");
  do{
    *ap++ = arg;
    arg = strtok(NULL," ");
  }while(arg != NULL);
  
  if(execvp(argv[0],argv))
    sh_printf("No command found!\n");
  
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
