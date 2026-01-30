#include <cpu/difftest.h>
#include <isa.h>
#include <memory/paddr.h>
#include <sim/diff_sim.h>
#include <stdint.h>

#ifdef CONFIG_DIFFTEST

DiffEssen diffEssen;

void diff_exec() {
  // Difftest Essential 遥测数据的获得
  getDiffEssen(&diffEssen);
#ifdef CONFIG_DIFFREG
  // Difftest Register 遥测数据的获得
  DiffReg diffReg;
  getDiffReg(&diffReg);                       // 获得提交信息
  setRegFile(&diffReg);                       // 设置包装器中的影子寄存器堆
  difftest_step(diffEssen.pc, diffEssen.npc); // 依赖影子寄存器堆进行差分
#endif
}

#endif /* ifdef CONFIG_DIFFTEST */
