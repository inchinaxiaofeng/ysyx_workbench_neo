#include "local-include/reg.h"
#include <isa.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// NOTE: 很好的设计, 32I和32E的前16个寄存器连名字都一样.
// 所以, 对于32E来说, 我们甚至不用改动这些东西. 太棒了.
// 悲哀, 之前我完全没有注意到过.
const char *regs[] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                      "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                      "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                      "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

void isa_reg_display() {
  print_split(GPR) for (size_t i = 0; i < sizeof(regs) / sizeof(char *); i++) {
    print_reg_val(GPR, regs[i], gpr(i)) if ((i + 1) % 4 == 0) {
      printf("\n");
      if ((i + 1) % 8 == 0)
        print_split(GPR)
    }
  }
  print_reg_val(GPR, "pc", cpu.pc);
  printf("\n");
  // TODO:
  // print_split(CSR)
  // print_reg_val(CSR, "mstatus", csr(MSTATUS))
  // print_reg_val(CSR, "mtvec", csr(MTVEC)) printf("\n");
  // print_reg_val(CSR, "mepc", csr(MEPC))
  // print_reg_val(CSR, "mcause", csr(MCAUSE)) printf("\n");
  // print_split(CSR)
  return;
}

word_t isa_reg_str2val(const char *s, bool *success) {

  *success = true;
  if (0 == strcmp(s, regs[0])) {
    return gpr(0);
  }
  if (0 == strcmp(s, "$pc")) {
    return cpu.pc;
  }
  for (size_t i = 1; i < 32; i++) {
    if (0 == strcmp(regs[i], s + 1)) {
      *success = true;
      return gpr(i);
    }
  }
  *success = false;
  return 0;
}
