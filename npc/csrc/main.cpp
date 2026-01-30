#include <common.h>
#include <stdio.h>
#include <utils.h>

void init_monitor(int, char *[]);
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[], char **env) {
  /* Initialize the monitor. */
  init_monitor(argc, argv);
  /* Start engine. */
  engine_start();
  return is_exit_status_bad();
}
