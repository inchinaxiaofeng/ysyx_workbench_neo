#include <common.h>
#include <engine/engine.h>
#include <macro.h>
#include <sim/verilator_sim.h>
#include <utils.h>

void engine_final() {
  // 获得遥测数据并进行输出.
  sim_final();
  return;
}
