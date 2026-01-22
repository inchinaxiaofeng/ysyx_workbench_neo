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

#include "../monitor/sdb/sdb.h"
#include "common.h"
#include "debug.h"
#include "macro.h"
#include "utils.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <elf.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

#ifdef CONFIG_FTRACE

#define JAL 0b1101111
#define JALR 0b1100111
#define FUNC_NAME_LENGTH 32
#define TAB_WIDTH 32

// NOTE: 所有这些都在 `monitor.c` 中定义
extern FMT_Elf_Sym *string_funcs; // func table
extern int string_func_count;     // string table中func的数量
extern char *str_tab;             // 段名 字符串表(ASCII字符串的堆积)

int call_num = 0; // 函数调用栈计数

void ftrace(Decode *s);                   // funct trace核心代码
void print_space(int n, int table_width); // 输出空格

#endif

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) {
    log_write("%s\n", _this->logbuf);
  }
#endif
  if (g_print_step) {
    IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
  }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
#ifdef CONFIG_WATCHPOINT
  if (check_watchpoint()) {
    nemu_state.state = NEMU_STOP;
  };
#endif /* ifdef CONFIG_WATCHPOINT */
  // NOTE: `IRINGBUF`
  IFDEF(CONFIG_IRINGBUF, iringbuf_load(_this));
  // NOTE: `FTRACE`
  IFDEF(CONFIG_FTRACE, ftrace(_this));
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst;
#ifdef CONFIG_ISA_x86
  for (i = 0; i < ilen; i++) {
#else
  for (i = ilen - 1; i >= 0; i--) {
#endif
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0)
    space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
              MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst,
              ilen);
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (; n > 0; n--) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING)
      break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
#ifdef CONFIG_FTRACE

  free(str_tab); // 释放内容
  str_tab = NULL;

#endif /*` ifdef CONFIG_FTRACE `*/
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0)
    Log("simulation frequency = " NUMBERIC_FMT " inst/s",
        g_nr_guest_inst * 1000000 / g_timer);
  else
    Log("Finish running in less than 1 us and can not calculate the simulation "
        "frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  // NOTE: `IRINGBUF`
#ifdef CONFIG_IRINGBUF
  iringbuf_display();
#endif /* ifdef CONFIG_IRINGBUF */
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
  case NEMU_END:
  case NEMU_ABORT:
  case NEMU_QUIT:
    printf("Program execution has ended. To restart the program, exit NEMU and "
           "run again.\n");
    return;
  default:
    nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
  case NEMU_RUNNING:
    nemu_state.state = NEMU_STOP;
    break;

  case NEMU_END:
  case NEMU_ABORT:
#ifdef CONFIG_IRINGBUF
    iringbuf_display();
#endif
    Log("nemu: %s at pc = " FMT_WORD,
        (nemu_state.state == NEMU_ABORT
             ? ANSI_FMT("ABORT", ANSI_FG_RED)
             : (nemu_state.halt_ret == 0
                    ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN)
                    : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
        nemu_state.halt_pc);
    // fall through
  case NEMU_QUIT:
    statistic();
  }
}

#ifdef CONFIG_IRINGBUF

char iringbuf[IRINGBUF_SIZE][128] = {0}; // 环形数组
uint64_t iringbuf_index = 0;             // 环形数组下标

void iringbuf_load(Decode *_this) {
  Assert(NULL != _this,
         "Func [iringbuf_load] requires argument that [Decode *_this].");
  strcpy(iringbuf[(iringbuf_index++) % IRINGBUF_SIZE], _this->logbuf);
  return;
}

void iringbuf_display() {
  for (size_t i = 0; i < IRINGBUF_SIZE; i++) {
    if (NULL == iringbuf[i] || strcmp(iringbuf[i], "") == 0)
      break;
    if (i == (iringbuf_index - 1) % IRINGBUF_SIZE)
      printf("\033[3;31m------>\033[0m\t");
    else
      printf("\t");
    printf("%s\n", iringbuf[i]);
  }
  return;
}

#endif /*` ifdef CONFIG_IRINGBUF `*/

#ifdef CONFIG_FTRACE

extern bool ftrace_en;

/*```
jal:
  |31    12|11 7|6     0|
  | offset | rd |1101111|
jalr:
  |31        20|19 15|14 12|11  7|6     0|
  |   offset   | rs1 | 000 | rd  |1100111|
ret:扩展为jalr x0, x1, 0
  |31        20|19 15|14 12|11  7|6     0|
  |000000000000|00001| 000 |00000|1100111|
```*/
void ftrace(Decode *inst) {
  if (!ftrace_en) {
    printf(ANSI_FG_RED " CONFIG_FTRACE was enabled but the -f option was not "
                       "specified. Please specify the -f option and run the "
                       "program again with an ELF format Img file." ANSI_NONE
                       "\n");
    return;
  }
  Decode s = *inst;
  uint32_t i = inst->isa.inst;

  // vaddr_t funct_i;
  // vaddr_t funct_j;

  bool jal = (BITS(i, 6, 0) == JAL);
  bool jalr = (BITS(i, 6, 0) == JALR);
  bool ret = i == 0x00008067;
  bool call = false; // 用于判断JAL或JALR指令是否是函数调用指令

  // 判断jal和jalr的跳转的位置是否是函数入口位置
  if (jal || jalr) {
    // 调用函数一般采用jal指令，有时也会用jalr指令；
    // ret指令：扩展为`jalr jalr x0, x1, 0`
    char *name = (char *)malloc(FUNC_NAME_LENGTH * sizeof(char)); // func 的名字
    memset(name, '\0', FUNC_NAME_LENGTH);

    for (size_t i = 0; i < string_func_count; i++) {
      if (s.dnpc == string_funcs[i].st_value) { // CALL:跳转到函数入口处
        call = true;
        for (size_t j = 0; j < FUNC_NAME_LENGTH &&
                           str_tab[string_funcs[i].st_name + j] != '\0';
             j++) {
          name[j] = str_tab[string_funcs[i].st_name + j];
        }
        /*
        测试结果：第一种实现方式：入口函数
          simulation frequency = 151493.7 inst/s（二十次平均）
        测试结果：第二种实现方式：范围判断
          simulation frequency = 117914.65 inst/s（二十次平均）
        */
        break;
      } else if (ret) { // RET:查找ret的目标
        if (s.dnpc >= string_funcs[i].st_value &&
            s.dnpc < string_funcs[i].st_value +
                         string_funcs[i]
                             .st_size) { // s->dnpc 属于 [value, value+size]
          for (size_t j = 0; j < FUNC_NAME_LENGTH &&
                             str_tab[string_funcs[i].st_name + j] != '\0';
               j++) {
            name[j] = str_tab[string_funcs[i].st_name + j];
          }
          break;
        }
      }
      // funct_i = string_funcs[i].st_value;
      // funct_j = string_funcs[i].st_value+string_funcs[i].st_size;
      // printf("%s:%d\n", "!ret",!ret);
      // printf("%s:%d\n", "jal|jalr",jal|jalr);
      // printf("%s:%d\n", "( (s->snpc >= funct_i) && (s->snpc < funct_j) )",
      //   ( (s->snpc >= funct_i) && (s->snpc < funct_j) ));
      // printf("%s:%d\n", "!( (s->dnpc >= funct_i) && (s->dnpc < funct_j) )",
      // !( (s->dnpc >= funct_i) && (s->dnpc < funct_j) ));
      // // printf("%s:%d\n", "!ret",!ret);
      // // printf("%s:%d\n", "!ret",!ret);

      // if (!ret
      //     && (jal|jalr)
      //     && ( (s->snpc >= funct_i) && (s->snpc < funct_j) )
      //     && !( (s->dnpc >= funct_i) && (s->dnpc < funct_j) )) { //
      //     跳转位置与当前位置不在一个函数域
      //   call = true;
      // }

      // if (s->dnpc >= funct_i && s->dnpc < funct_j) { // s->dnpc 属于 [value,
      // value+size]
      //   for (size_t j = 0; j < FUNC_NAME_LENGTH &&
      //   str_tab[string_funcs[i].st_name+j] != '\0'; j++)
      //   {
      //     name[j] = str_tab[string_funcs[i].st_name+j];
      //   }
      //   break;
      // }
    }

    // 输出
    if (call) {
      if (0 == call_num) {
        printf(ANSI_FG_GREEN "0x" ANSI_NONE "80000000:[" ANSI_FG_MAGENTA
                             "entry" ANSI_NONE "]\n");
      }
      call_num++;
      Assert(call_num >= 0,
             "CALL:Call_num out of range. Check me in "
             "[cpu-exec.c]. Call_num %d",
             call_num);
      printf(ANSI_FG_YELLOW FMT_WORD ":" ANSI_NONE, s.pc);
      print_space(call_num, 2);
      printf(ANSI_BG_YELLOW "call" ANSI_NONE " [" ANSI_FG_MAGENTA "%s" ANSI_NONE
                            "@" FMT_WORD "]\n",
             name, s.dnpc);
    } else if (ret) {
      call_num--;
      Assert(call_num >= 0,
             "RET:Call_num out of range. Check me in [cpu-exec.c]. Call_num %d",
             call_num);
      printf(ANSI_FG_GREEN FMT_WORD ":" ANSI_NONE, s.pc);
      print_space(call_num, 2);
      printf(ANSI_BG_GREEN "ret" ANSI_NONE "  [" ANSI_FG_MAGENTA "%s" ANSI_NONE
                           "]\n",
             name);
    }

    // 释放
    free(name);
    name = NULL;
  }

  return;
}

void print_space(int n, int table_width) {
  if (n > TAB_WIDTH) {
    printf("\033[3;36m");
    for (size_t i = 0; i < TAB_WIDTH * table_width; i++) {
      printf(" ");
    }
    printf("TAB:%d\033[0m", n);
    return;
  }
  for (size_t i = 0; i < n * table_width; i++) {
    printf(" ");
  }
  return;
}
#endif
