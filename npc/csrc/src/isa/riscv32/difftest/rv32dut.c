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

#include "../local-include/reg.h"
#include "utils.h"
#include <cpu/difftest.h>
#include <isa.h>

extern const char *regs[32];

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if (memcmp(ref_r, &cpu, sizeof(vaddr_t) * 33) == 0) {
    return true;
  } else {
    difftest_display_ref(ref_r);
    return false;
  }
}

void difftest_display_ref(CPU_state *ref_r) {
  printf(ANSI_FG_RED "DIFFTEST" ANSI_NONE "\n");
  printf(ANSI_FG_RED "REF" ANSI_NONE "\n");
  print_split(GPR) for (size_t i = 0; i < sizeof(regs) / sizeof(char *); i++) {
    print_reg_val(GPR, regs[i], ref_r->gpr[i]) if ((i + 1) % 4 == 0) {
      printf("\n");
      if ((i + 1) % 8 == 0)
        print_split(GPR)
    }
  }
  print_reg_val(GPR, "pc", ref_r->pc);
  printf("\n");
  // printf("\033[3;36m%3s:\033[0m \033[2;32m0x\033[0m%016lx\t\n", "PC",
  //        ref_r->pc);
  // printf("\033[3;34m%7s:\033[0m \033[2;32m0x\033[0m%016lx\t\n", "mepc",
  //        ref_r->csr[0]);
  // printf("\033[3;34m%7s:\033[0m \033[2;32m0x\033[0m%016lx\t\n", "mstatus",
  //        ref_r->csr[1]);
  // printf("\033[3;34m%7s:\033[0m \033[2;32m0x\033[0m%016lx\t\n", "mcause",
  //        ref_r->csr[2]);
  // printf("\033[3;34m%7s:\033[0m \033[2;32m0x\033[0m%016lx\t\n", "mtvec",
  //        ref_r->csr[3]);
}

void isa_difftest_attach() {}
