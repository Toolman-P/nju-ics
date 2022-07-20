#include <cpu/cpu.h>
#include <cpu/exec.h>
#include <cpu/difftest.h>
#include <isa-all-instr.h>
#include <locale.h>
/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 15
#define MAX_IRING_BUF 15

CPU_state cpu = {};
uint64_t g_nr_guest_instr = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
const rtlreg_t rzero = 0;
rtlreg_t tmp_reg[4];

#if CONFIG_ITRACE
static struct {
  uint8_t pos;
  char bufs[MAX_IRING_BUF][128];
} iring;

void init_iringbuf(){
  iring.pos=0;
  memset(iring.bufs,0,sizeof(iring.bufs));
}

void copy_buf(char *p){
  strcpy(iring.bufs[iring.pos],p);
  iring.pos = (iring.pos+1)%MAX_IRING_BUF;
}

void print_iringbuf(){
  printf("-----------------------------\n");
  printf("------  IRING   TRACE  ------\n");
  printf("-----------------------------\n");
  for(uint8_t i=0;i<MAX_IRING_BUF;i++){
    if(iring.pos==i)
      printf(ASNI_FMT("-->\n",ASNI_FG_GREEN));
    printf("%s\n",iring.bufs[i]);
  }
}

  #if CONFIG_FTRACE
    uint64_t stack_dep=0;
    extern bool is_ftraceopen;
  #endif
#endif

void device_update();
void fetch_decode(Decode *s, vaddr_t pc);
void diff_watchpoints();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND){
    #if CONFIG_FTRACE
      for(int i=0;i<stack_dep;i++)
        log_write(" ");
    #endif
    log_write("%s\n", _this->logbuf);
    diff_watchpoints();
  }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
}

#include <isa-exec.h>

#define FILL_EXEC_TABLE(name) [concat(EXEC_ID_, name)] = concat(exec_, name),
static const void* g_exec_table[TOTAL_INSTR] = {
  MAP(INSTR_LIST, FILL_EXEC_TABLE)
};

static void fetch_decode_exec_updatepc(Decode *s) {
#if CONFIG_TRACE
  #if CONFIG_ETRACE
    if((s->isa.instr.val & 0x7f) == 0x73){
      switch(s->isa.instr.val){
        case 0x73:
          log_write(ASNI_FMT("[EXP] Triggered ecall intr \n",ASNI_FG_RED));
          break;
        case 0x30200073:
          log_write(ASNI_FMT("[EXP] Triggered mret intr \n",ASNI_FG_BLUE));
          break;
      }
    }
  #endif
#endif
  fetch_decode(s, cpu.pc);
  s->EHelper(s);
  cpu.pc = s->dnpc;

#if CONFIG_TRACE
  #ifdef CONFIG_FTRACE
    if(is_ftraceopen){
      char *search_symbol(word_t pc);
      if(s->isa.instr.val == 0x00008067){
        if(stack_dep){
          stack_dep--;
          for(int i=0;i<stack_dep;i++)
            log_write(" ");
          log_write(ASNI_FMT(FMT_WORD": ret\n",ASNI_FG_BLUE),s->pc);
        }
      }else if((s->isa.instr.val & 0xff) == 0xef || (s->isa.instr.val & 0xff) == 0xe7){
        if(s->dnpc!=s->snpc){
          char *symbol=search_symbol(s->dnpc);
          if(symbol){
            for(int i=0;i<stack_dep;i++)
            log_write(" ");
            log_write(ASNI_FMT(FMT_WORD": call [%s@"FMT_WORD"]\n",ASNI_FG_BLUE),s->pc,symbol,s->dnpc);  
            stack_dep++;
          }
        }
      }
    }
  #endif
#endif
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%ld", "%'ld")
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_instr);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " instr/s", g_nr_guest_instr * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
  #ifdef CONFIG_ITRACE
  print_iringbuf();
  #endif
}

void fetch_decode(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  int idx = isa_fetch_decode(s);
  s->dnpc = s->snpc;
  s->EHelper = g_exec_table[idx];
#if CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *instr = (uint8_t *)&s->isa.instr.val;
  for (i = 0; i < ilen; i ++) {
    p += snprintf(p, 4, " %02x", instr[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.instr.val, ilen);
  copy_buf(p);

#endif
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INSTR_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  Decode s;
  for (;n > 0; n --) {
    fetch_decode_exec_updatepc(&s);
    g_nr_guest_instr ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ASNI_FMT("ABORT", ASNI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ASNI_FMT("HIT GOOD TRAP", ASNI_FG_GREEN) :
            ASNI_FMT("HIT BAD TRAP", ASNI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
