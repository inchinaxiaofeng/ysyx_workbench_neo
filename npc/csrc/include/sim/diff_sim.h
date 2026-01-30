#ifndef __DIFF_SIM_H__
#define __DIFF_SIM_H__

#include "common.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool valid;
  word_t pc;
  word_t npc;
  uint32_t inst;
  bool isRVC;
} DiffEssen;
bool getDiffEssen(DiffEssen *);

typedef struct {
  bool wen;
  int wdest; // 在32E中取[0, 15], 其他情况下是[0, 31]
  word_t wdata;
} DiffReg;
bool getDiffReg(DiffReg *);

#endif // !__DIFF_SIM_H__
