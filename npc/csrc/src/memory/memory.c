#include <isa.h>
#include <memory/memory.h>
#include <memory/paddr.h>
#include <sim/mem_sim.h>
#include <stdint.h>

SimMem simMem;

int countSetBits(uint32_t n) {
  int count = 0;
  while (n > 0) {
    n &= (n - 1);
    count++;
  }
  return count;
}

void memory_exec() {
  getSimMem(&simMem);
  if (simMem.iRen) {
    setSimMem(paddr_read(simMem.iReadAddr, 32));
  }
  if (simMem.iWen) {
    int len = countSetBits(simMem.iByteMask);
    assert(8 == len || 16 == len || 32 == len);
    paddr_write(simMem.iWriteAddr, len, simMem.iWriteData);
  }
  return;
}
