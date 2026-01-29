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

#include <cpu/cpu.h>
#include <difftest-def.h>
#include <isa.h>
#include <memory/paddr.h>
#include <string.h>

// 在 DUT host memory 的 buf 和REF guest memory 的addr 之间拷贝 n 字节,
// direction 指定拷贝方向,
// DIFFTEST_TO_DUT 表示向DUT拷贝, DIFFTEST_TO_REF 表示向REF拷贝
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n,
                              bool direction) {
  if (DIFFTEST_TO_REF == direction) {
    memcpy(guest_to_host(addr), buf, n);
  } else if (DIFFTEST_TO_DUT == direction) {
    memcpy(buf, guest_to_host(addr), n);
  }
  return;
}

// `direction` 为 `DIFFTEST_TO_DUT` 时, 获取REF的寄存器状态到`dut`
// `direction` 为 `DIFFTEST_TO_REF` 时, 设置REF的寄存器状态为`dut`
__EXPORT void difftest_regcpy(void *dut, bool direction) {
  if (DIFFTEST_TO_REF == direction) {
    memcpy(&cpu, dut, DIFFTEST_REG_SIZE);
  } else if (DIFFTEST_TO_DUT == direction) {
    memcpy(dut, &cpu, DIFFTEST_REG_SIZE);
  }
  return;
}

__EXPORT void difftest_exec(uint64_t n) { cpu_exec(n); }

__EXPORT void difftest_raise_intr(word_t NO) {
  cpu.pc = isa_raise_intr(NO, cpu.pc);
  assert(0);
}

__EXPORT void difftest_init(int port) {
  // 由于`pmem`是一个数组, 初始就是0. 这里的init是模拟存储空间的"随机"的.
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
}
