/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"
#include "common.h"
#include "memory/paddr.h"
#include "utils.h"
#include <cpu/cpu.h>
#include <ctype.h>
#include <isa.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int cmd_c(char *args);
static int cmd_q(char *args);
static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si",
     "[si N]. Execute N instructions in a single step and then pause "
     "execution. When N is not given, the default is 1",
     cmd_si},
    {"info",
     "[info SUBCMD]. Print [r]egister status. Print [w]atch point info. Print "
     "[a]ll if no arg given.",
     cmd_info},
    {"x",
     "[x N EXPR]. Figure out the value of EXPR, use it as the starting mem "
     "address, then output the result in hex form as N consecutive 4-bytes",
     cmd_x},
    {"p", "[p EXPR]. Figure out the value of EXPR.", cmd_p},
    {"w",
     "[w EXPR]. Pause program execution when the value of the expression EXPR "
     "changes.",
     cmd_w},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) { return -1; }

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

static int cmd_si(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  uint64_t n;

  if (NULL == arg)
    n = 1; /* no argument given */
  else {
    // 判断字符串中所有字符是否都是数字
    for (size_t i = 0; '\0' != arg[i]; i++) {
      if (!isdigit(arg[i])) {
        printf(ANSI_FG_RED "[SDB] The parameter is illegal and requires the "
                           "parameter [N] to be of "
                           "type FMT_WORD in decimal." ANSI_NONE "\n");
        return 0;
      }
    }
    // 将字符串转换为 `uint64_t` 型的数字
    n = strtoul(arg, NULL, 10);
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  /* No argument given, print all info. */
  if (NULL == arg) {
    isa_reg_display();
    watchpoint_display();
    return 0;
  } else if (!isalpha(arg[0])) { // 合法性判断
    printf(
        ANSI_FG_RED
        "[SDB] The parameter is inllegal and requires the first character of "
        "[SUBCMD] is alphabet." ANSI_NONE "\n");
    return 0;
  }
  // 获取 `arg` 第一个参数的第一个字符作为选项
  switch (arg[0]) {
  case 'r':
    isa_reg_display();
    break;
  case 'w':
    watchpoint_display();
    break;
  default:
    printf(ANSI_FG_RED "[SDB] Unknown option." ANSI_NONE "\n");
    break;
  }
  return 0;
}

static int cmd_x(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  if (NULL == arg) {
    printf(ANSI_FG_RED "[SDB] No exprs are give." ANSI_NONE "\n");
    return 0;
  }
  // 获取使用 `sscanf` 获取 `num` 和 `base`
  uint64_t num;
  paddr_t base = CONFIG_MBASE; // NOTE: 使用基地址开始计算
  bool success = true;
  base = expr(arg + strlen(arg) + 1, &success);
  if (!sscanf(arg, "%lu", &num) || !success) {
    printf(ANSI_FG_RED "The arguments should be given as: " ANSI_FG_YELLOW
                       "x [N] [EXPR]." ANSI_NONE "\n");
    return 0;
  }
  if (!success) {
    printf(ANSI_FG_RED "Expr() not success." ANSI_NONE "\n");
    return 0;
  }
  // NOTE: 在一些情况下, 硬件环境可能不会使用0x8000_0000 作为基地址.
  // 因为没有进一步的考虑, 所以将这种情况视为错误.
  if (0 == num || 0 == base) {
    printf(ANSI_FG_RED "The arguments is illegal. Num: %lu, Base: %x" ANSI_NONE
                       "\n",
           num, base);
    return 0;
  }

  for (size_t i = 0; i < num; i++) {
    if (0 == i % 4) {
      // NOTE: 将地址值强制转换为`void*`类型, 并使用`%p`格式化输出,
      // 可以避免因为使用错误的格式化字符串导致的安全问题
      printf("\n" ANSI_FG_YELLOW "[INFO] " ANSI_FG_CYAN "%p: \t",
             (void *)(base + i * 4));
    }

    for (size_t j = 0; j < 4; j++) {
      // FIXME: 当我输入的地址不合法时将会直接崩溃
      uint8_t *pos = guest_to_host(base + i * 4 + j);
      printf(ANSI_NONE "%.2x ", *pos);
    }
    printf("\t");
  }
  printf("\n");
  return 0;
}

// FIXME: 如果解奇怪的地址, 就会导致`NEMU`崩溃
static int cmd_p(char *args) {
  /* Extract the first argument */
  // NOTE: 这里之所以不进行分割, 是因为后面的`expr`处理是以`\0`为结尾的.
  if (NULL == args) {
    printf(ANSI_FG_RED "No expr is given." ANSI_NONE "\n");
    return 0;
  }

  bool success = true;
  word_t result = expr(args, &success);
  if (!success) {
    Log(ANSI_FG_RED "Expr() not success." ANSI_NONE);
    return 0;
  }
  printf(ANSI_FG_YELLOW "DEC:" ANSI_NONE " = 0d" FMT_PRId "\n" ANSI_FG_YELLOW
                        "HEX:" ANSI_NONE " = 0x" FMT_PRIx "\n",
         result, result);
  return 0;
}

static int cmd_w(char *args) {
  /* extract the first argument */
  if (NULL == args) {
    printf(ANSI_FG_RED "No arg is given." ANSI_NONE "\n");
    return 0;
  }
  bool success = true;
  WP *point = new_wp(args, &success);
  if (!success) {
    printf(ANSI_FG_RED "Some thing wrong happend!" ANSI_NONE "\n");
    free_wp_NO(point->NO);
  } else {
    printf("Create a " ANSI_FG_CYAN "WatchPoint(NO.%d)" ANSI_NONE ": %s\n",
           point->NO, point->condation);
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
