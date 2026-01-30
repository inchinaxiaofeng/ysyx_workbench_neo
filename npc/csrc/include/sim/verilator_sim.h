#ifndef __VERILATOR_SIM_H__
#define __VERILATOR_SIM_H__

#include <autoconfig.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

extern bool trace_en;
extern bool tele_diff_essen;
extern bool tele_diff_reg;

void single_step();

#ifdef CONFIG_VCD_TRACE
#define TRACE_DUMP(cond, tfp, time)                                            \
  if (cond) {                                                                  \
    tfp->dump(time);                                                           \
  }
#else
#define TRACE_DUMP(cond, tfp, time)
#endif

#endif // !__VERILATOR_SIM_H__
