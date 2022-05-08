#include "common.h"
#include "debug.h"
#include "memory/paddr.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdint.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NUM_DEC,
  TK_NUM_HEX,
  TK_UEQ,
  TK_AND,
  TK_OR,
  TK_NEG,
  TK_DEREF,
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {"\\$ra", '$'},
    {"\\$sp", '$'},
    {"\\$gp", '$'},
    {"\\$tp", '$'}, /* register defined in the src/isa/riscv64(32)/reg.c */
    {"\\$a[0-7]", '$'},
    {"\\$t[0-6]", '$'},
    {"\\$s1[0-1]", '$'},
    {"\\$s[0-9]", '$'},

    {" +", TK_NOTYPE},                   // spaces
    {"\\(", '('},                        // left parentise
    {"\\)", ')'},                        // right parentise
    {"(0x|0X)[0-9A-Fa-f]+", TK_NUM_HEX}, // hexidemal numbers
    {"[0-9]+", TK_NUM_DEC},              // decimal numbers
    {"\\+", '+'},                        // plus
    {"-", '-'},                          // minus
    {"\\*", '*'},                        // multiply
    {"/", '/'},                          // divide
    {"==", TK_EQ},                       // equal
    {"!=", TK_UEQ},                      // uneuqal
    {"&&", TK_AND},                      // and
    {"\\|\\|", TK_OR},                   // or
    {"\\$\\$0", '$'},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_NOTYPE: // No need to save tokens for pure blanks
          break;
        case '$':
          substr_start++;
          substr_len--;
        default:
          tokens[nr_token].type = rules[i].token_type;
          for (int i = 0; i < substr_len; i++)
            tokens[nr_token].str[i] = *(substr_start + i);
          nr_token++;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int check_parentheses(int l, int r) {
  int left = 256, right = -1;
  int cnt = 0; // the count of left parenthese
  for (int i = l; i <= r; i++) {
    if (tokens[i].type == '(') {
      if (i < left)
        left = i;
      cnt++;
    } else if (tokens[i].type == ')') {
      if (i > right)
        right = i;
      if (cnt == 0)
        panic("Isolated right parenthese.");
      else
        cnt--;
    }
  }
  if (cnt > 0)
    panic("Isolated left parenthese detected.");
  if (left == l && right == r)
    return 1;
  return 0;
}

bool check_op(int pos) {
  return tokens[pos].type != TK_NUM_DEC && tokens[pos].type != TK_NUM_HEX &&
         tokens[pos].type != '(' && tokens[pos].type != ')';
}

int eval_priority(int pos) {
  switch (tokens[pos].type) {
  case '$':
    return 6;
  case TK_NEG:
  case TK_DEREF:
    return 5;
  case '*':
  case '/':
    return 4;
  case '+':
  case '-':
    return 3;
  case TK_EQ:
  case TK_UEQ:
    return 2;
  case TK_AND:
    return 1;
  case TK_OR:
    return 0;
  default:
    assert(0);
  }
}

int find_op_pos(int l, int r) {
  int pos = l;
  int priority = 256;
  for (int i = l; i <= r; i++)
    if (check_op(i)) {
      int next_priority = eval_priority(i);
      if (next_priority < priority) {
        priority = next_priority;
        pos = i;
      }
    }
  return pos;
}

word_t eval(int l, int r) {

  if (tokens[l].type == TK_DEREF) {
    word_t val = eval(l + 1, r);
    return *guest_to_host(val);
  } else if (tokens[l].type == TK_NEG) {
    return -eval(l + 1, r);
  }

  if (l > r) {
    panic("Some illegal expressions detected");
  } else if (l == r) {
    switch (tokens[l].type) {
    case TK_NUM_DEC:
      return strtoll(tokens[l].str, NULL, 10);
    case TK_NUM_HEX:
      return strtoll(tokens[l].str, NULL, 16);
    case '$':
      return isa_reg_str2val(tokens[l].str, NULL);
    default:
      panic("Not a numeric type.");
    }
  } else if (check_parentheses(l, r)) {
    return eval(l + 1, r - 1);
  } else {

    int op = find_op_pos(l, r);
    word_t val1 = eval(l, op - 1);
    word_t val2 = eval(op + 1, r);
    switch (tokens[op].type) {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
      if (val2 == 0) {
        panic("Error: Divided By Zero.");
      }
      return val1 / val2;
    case TK_EQ:
      return val1 == val2;
    case TK_UEQ:
      return val1 != val2;
    case TK_AND:
      return val1 && val2;
    case TK_OR:
      return val1 || val2;
    default:
      panic("Certain op type not supported.");
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  for (int i = 0; i < nr_token; i++) {
    if (i == 0 || !(tokens[i - 1].type == TK_NUM_DEC ||
                    tokens[i - 1].type == TK_NUM_HEX)) {
      if (tokens[i].type == '-')
        tokens[i].type = TK_NEG;
      else if (tokens[i].type == '*')
        tokens[i].type = TK_DEREF;
    }
  }

  *success = true;
  return eval(0, nr_token - 1);
}
