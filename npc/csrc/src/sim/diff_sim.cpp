#include <common.h>
#include <sim/diff_sim.h>
#include <sim/verilator_sim.h>
#include <stdbool.h>
#include <stdio.h>
#include str(VSimTop_H)

extern VSimTop *sim_top;

bool getDiffEssen(DiffEssen *diffEssen) {
  if (NULL == diffEssen) {
    return false;
  }
  diffEssen->valid = sim_top->diffEssenIO_valid;
  diffEssen->pc = sim_top->diffEssenIO_pc;
  diffEssen->npc = sim_top->diffEssenIO_npc;
  diffEssen->inst = sim_top->diffEssenIO_inst;
  diffEssen->isRVC = sim_top->diffEssenIO_isRVC;

  return true;
}

bool getDiffReg(DiffReg *diffReg) {
  if (NULL == diffReg) {
    return false;
  }
  diffReg->wen = sim_top->diffRegIO_wen;
  diffReg->wdest = sim_top->diffRegIO_wdest;
  diffReg->wdata = sim_top->diffRegIO_wdata;

  return true;
}
