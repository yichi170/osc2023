#ifndef _DEVTREE_H
#define _DEVTREE_H

#include "type.h"

struct fdt_header {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
};

struct fdt_prop {
  uint32_t len;
  uint32_t nameoff;
};

#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009

#define DTB_MAGIC       0xD00DFEED

extern uint64_t DTB_ADDR;

uint32_t u32_to_little_endian(uint32_t);
void fdt_traverse(void (*)());

#endif
