#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "common.h"
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  int steps=strtol(args,NULL,10);
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args){
  
  if(args==NULL){
    printf("Please pass through subcommands");
    return 0;
  }
  
  char *cmd=strtok(args," ");
  if(cmd[0]=='r'){
    isa_reg_display();
  }else{
    printf("INFO only supports subcommands(r/w)\n");
  }
  return 0;
}

static int cmd_x(char *args){

  char *args_end = args+strlen(args);

  if(args==NULL){
    printf("Not Enough Arguments\n");
    return 0;
  }

  char *Bytes = strtok(args," ");
  int bytes = strtol(Bytes,NULL,10);
  args = args + strlen(Bytes) + 1;

  if(args>=args_end){
    printf("Not Enough Arguments.\n");
    return 0;
  }

  char *Addr = strtok(args," ");
  word_t addr = strtoll(Addr,NULL,16);

  printf("---------MEMORY----------\n");
  printf("Total Bytes: %d\n",bytes);
  printf("Starting Addr: 0X%016lX\n",addr);
  printf("Mem: "); 

  char *pt = (char *) addr;
  for(int i=0;i<bytes;i++){
    printf("%X ",*(pt+i));
  }

  printf("\n");
  printf("-----------END----------\n");
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step Into", cmd_si},
  { "info", "Info status (r/w)", cmd_info},
  { "x", "Print N bytes starting from the address", cmd_x}

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
