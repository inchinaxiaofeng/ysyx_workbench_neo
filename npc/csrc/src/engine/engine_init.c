#include <cpu/exec.h>
#include <monitor/monitor.h>
#include <monitor/sdb.h>
void sdb_mainloop();

void engine_start() {
  /* Receive commands from user. */
  sdb_mainloop();
}
