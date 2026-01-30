#include "common.h"
#include <local-include/reg.h>
#include <memory/paddr.h>
#include <sim/verilator_sim.h>

#define R(i) gpr(i)
#define PC cpu_pc

static const uint32_t img[] = {
    0x00000297, 0x0002b823, 0x0102b503, 0x00100073, 0xdeadbeef,
};

static void restart() {
  /* Set the initial program counter. */
  Log("Should Restart PC");
}

void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  restart();
}

void setRegFile(word_t rd, word_t wdata, word_t npc) {
  PC = npc;
  R(rd) = wdata;
  return;
}
