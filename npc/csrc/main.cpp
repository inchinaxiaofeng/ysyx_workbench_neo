#include <common.h>
#include <engine/engine.h>
#include <stdio.h>
#include <utils.h>

void init_monitor(int, char *[]);
int is_exit_status_bad();

int main(int argc, char *argv[], char **env) {
  /* Initialize the monitor. */
  init_monitor(argc, argv);
  /* Start engine. */
  engine_init();
  engine_start();
  return is_exit_status_bad();
}
