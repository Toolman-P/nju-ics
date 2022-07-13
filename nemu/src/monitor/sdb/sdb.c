#include <cpu/cpu.h>
#include <isa.h>
#include <memory/paddr.h> 
#include <readline/history.h>
#include <readline/readline.h>

#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
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

static int cmd_q(char *args) { return -1; }

static int cmd_help(char *args);

static int cmd_si(char *args) {
  if(!args) 
    cpu_exec(1);
  else
    cpu_exec(atoi(args));
  return 0;
}

static int cmd_info(char *args) {

  if (args == NULL) {
    printf("Please pass through subcommands.\n");
    return 0;
  }

  char *cmd = strtok(args, " ");
  if (cmd[0] == 'r') {
    isa_reg_display();
  } else if (cmd[0]=='w'){
    print_watchlist();    
  }
  return 0;
}

static int cmd_x(char *args) {

  char *args_end = args + strlen(args);

  if (args == NULL) {
    printf("Not Enough Arguments.\n");
    return 0;
  }

  char *Bytes = strtok(args, " ");
  int bytes = strtol(Bytes, NULL, 10);
  args = args + strlen(Bytes) + 1;

  if (args >= args_end) {
    printf("Not Enough Arguments.\n");
    return 0;
  }

  char *Addr = strtok(args, " ");
  word_t addr = strtoll(Addr, NULL, 16);

  printf("---------MEMORY----------\n");
  printf("Total Bytes: %d\n", bytes);
  printf("Starting Addr::" FMT_WORD"\n", addr);
  printf("Mem: ");
  for (int i = 0; i < bytes; i++)
    printf("0x%02x ", *guest_to_host(addr + i));
  printf("\n");
  printf("-----------END-----------\n");
  return 0;
}

word_t eval_expr(char *args){
  if(args == NULL){
    panic("Please provide an expression.");
  }

  static bool success;
  word_t val = expr(args,&success);

  if (success){
    return val;
  }else{
    panic("No Regex match.");
  }
}

static int cmd_p(char *args) {
  printf(FMT_WORD"\n", eval_expr(args));
  return 0;
}

static int cmd_w(char *args){
  
  assert(strlen(args)<=NR_EX);

  WP *wp=new_wp();
  wp->value=eval_expr(args);
  strcpy(wp->expression,args);

  return 0;
}

static int cmd_d(char *args){

  int no = strtol(args,NULL,10);
  WP *wp = find_wp(no);
  free_wp(wp);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Step Into", cmd_si},
    {"info", "Info status (r/w)", cmd_info},
    {"x", "Print N bytes starting from the address", cmd_x},
    {"p", "Eval the result of the expression.", cmd_p},
    {"w", "Set watchpoint on the expression", cmd_w},
    {"d", "Delete watchpoints", cmd_d},

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}


void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

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
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
