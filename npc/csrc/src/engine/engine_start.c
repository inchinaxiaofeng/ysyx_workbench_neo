
#include "debug.h"
void sdb_mainloop();

void engine_start() {
  LogInfo("[ENGINE] Engine Loop Start.");
  /* Receive commands from user. */
  sdb_mainloop();
}
