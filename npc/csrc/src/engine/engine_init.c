#include <cpu/exec.h>
#include <engine/engine.h>
#include <monitor/monitor.h>
#include <monitor/sdb.h>

void engine_start() {
  cpu_init();
  sdb_mainloop();
  final();
}
