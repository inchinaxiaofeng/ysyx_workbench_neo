#include <common.h>
#include <engine/engine.h>
#include <isa.h>
#include <memory/host.h>
#include <memory/paddr.h>
#include <stdint.h>
#include <stdio.h>
#include <utils.h>

#if defined(PMEM_MALLOC)
static uint8_t pmem = NULL;
#else
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

extern CPU_state env_cpu;

uint8_t *guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  engine_final();
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR
        ", " FMT_PADDR "] at pc = " FMT_WORD,
        addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if defined(MEM_MALLOC)
  pmem = malloc(MSIZE);
  assert(pmem);
#endif
#ifdef MEM_RANDOM
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int)(CONFIG_MSIZE / sizeof(p[0])); i++) {
    p[i] = rand();
  }
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT,
      PMEM_RIGHT);
}

#ifdef CONFIG_MTRACE
bool mtrace_en = false;

void static mtrace_display(paddr_t addr, int len, word_t data, bool is_write) {
  // NOTE: 全局使能信号关闭. 请在`sdb`运行中启动.
  if (false == mtrace_en) {
    return;
  }

  if (true == is_write)
    printf(ANSI_FG_MAGENTA "write:" ANSI_NONE);
  else
    printf(ANSI_FG_MAGENTA "read:" ANSI_NONE);

  printf(ANSI_BG_GREEN "paddr_t:" ANSI_NONE "\t" ANSI_BG_YELLOW "HEX:" ANSI_NONE
                       " " ANSI_BG_GREEN "0x" ANSI_NONE FMT_WORD "\t",
         addr);
  printf(ANSI_BG_GREEN "len:" ANSI_NONE "\t\t" ANSI_BG_YELLOW "DEC:" ANSI_NONE
                       " %d\n",
         len);
  if (true == is_write)
    printf(ANSI_BG_GREEN "data:" ANSI_NONE "\t\t" ANSI_BG_YELLOW
                         "DEC:" ANSI_NONE FMT_WORD " \n",
           data);

  printf("\n");
  return;
}
#endif /* ifdef CONFIG_MTRACE */

word_t paddr_read(paddr_t addr, int len) {
  IFDEF(CONFIG_MTRACE, mtrace_display(addr, len, 0, false));
  if (likely(in_pmem(addr)))
    return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  IFDEF(CONFIG_MTRACE, mtrace_display(addr, len, data, true));
  if (likely(in_pmem(addr))) {
    pmem_write(addr, len, data);
    return;
  }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
