#ifndef __CPU_DIFFTEST_H__
#define __CPU_DIFFTEST_H__

#include <common.h>
#include <difftest-def.h>

#ifdef CONFIG_DIFFTEST
void difftest_step(vaddr_t pc, vaddr_t npc);
#endif // CONFIG_DIFFTEST

#endif // !__CPU_DIFFTEST_H__
