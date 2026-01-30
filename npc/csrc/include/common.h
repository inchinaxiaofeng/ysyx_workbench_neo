/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <autoconfig.h>
#include <macro.h>

#ifdef CONFIG_TARGET_AM
#include <klib.h>
#else
#include <assert.h>
#include <stdlib.h>
#endif

#if CONFIG_MBASE + CONFIG_MSIZE > 0x100000000ul
#define PMEM64 1
#endif

typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t) sword_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)
#define FMT_PRIx MUXDEF(CONFIG_ISA64, "%" PRIx64, "%" PRIx32)
#define FMT_PRId MUXDEF(CONFIG_ISA64, "%" PRId64, "%" PRId32)

typedef word_t vaddr_t;
typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)
typedef uint16_t ioaddr_t;

#include <debug.h>

#ifdef CONFIG_FTRACE

#include <elf.h>
#define FMT_Elf_Sym MUXDEF(CONFIG_ISA64, Elf64_Sym, Elf32_Sym)
#define FMT_Elf_Ehdr MUXDEF(CONFIG_ISA64, Elf64_Ehdr, Elf32_Ehdr)
#define FMT_Elf_Shdr MUXDEF(CONFIG_ISA64, Elf64_Shdr, Elf32_Shdr)
#define FMT_ELF_ST_TYPE MUXDEF(CONFIG_ISA64, ELF64_ST_TYPE, ELF32_ST_TYPE)

#endif // CONFIG_FTRACE

#endif
