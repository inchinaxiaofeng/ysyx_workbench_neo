#include "debug.h"
#include <sim/verilator_sim.h>

void engine_init() {
  LogInfo("[ENGINE] Initial Wapper Engine environment...");
  sim_init();
  LogOK("[ENGINE] Wapper Engine environment ready.");
  return;
}
