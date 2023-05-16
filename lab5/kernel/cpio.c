#include "cpio.h"
#include "thread.h"
#include "process.h"
#include "string.h"
#include "print.h"
#include "devtree.h"
#include "mm_utils.h"

#define CPIO_HEADER_MAGIC "070701"

extern void core_timer_enable();
extern void from_el1_to_el0(unsigned int, unsigned int);
static void *INITRD_ADDR = 0;
static void *INITRD_END = 0;
void *initrd_end = 0;

void get_ramfs_addr(const char *nodename, const char *propname, void *prop_val) {
  if (streq(nodename, "chosen") == 0 &&
      streq(propname, "linux,initrd-start") == 0) {
    uint32_t initrd_addr = u32_to_little_endian(*(uint32_t *)prop_val);
    INITRD_ADDR = (void *)initrd_addr;
    logdf("initrd addr: %#X\n", INITRD_ADDR);
  }
  if (streq(nodename, "chosen") == 0 &&
      streq(propname, "linux,initrd-end") == 0) {
    uint32_t initrd_addr = u32_to_little_endian(*(uint32_t *)prop_val);
    INITRD_END = (void *)initrd_addr;
    initrd_end = INITRD_END;
    logdf("initrd addr (end): %#X\n", INITRD_END);
  }
  return;
}

void reserve_ramfs() {
  if (INITRD_ADDR == 0 || INITRD_END == 0) {
    printf("[WARN] cannot reserve memory for ramfs\n");
    return;
  }
  memory_reserve(INITRD_ADDR, INITRD_END);
}

__attribute__ ((always_inline))
inline uint32_t get_file_size(cpio_newc_header_t *cpio_header) {
  return strtoui(cpio_header->c_filesize, 8, 16);
}

__attribute__ ((always_inline))
inline uint32_t get_name_size(cpio_newc_header_t *cpio_header) {
  return strtoui(cpio_header->c_namesize, 8, 16);
}

void read_filename(char *cpio_addr, char *filename, int len) {
  char *fname_addr = cpio_addr + sizeof(cpio_newc_header_t);
  int i;
  for (i = 0; i < len; i++) {
    filename[i] = fname_addr[i];
  }
  filename[i] = '\0';
}

char *cpio_list_one(char *addr, char *filename) {
  char *addrptr = addr;
  cpio_newc_header_t *file_header = (cpio_newc_header_t *) addr;

  if (strneq(file_header->c_magic, CPIO_HEADER_MAGIC, 6) != 0) {
    logdf("Expected magic: %#X, your magic: %#X\n", CPIO_HEADER_MAGIC, file_header->c_magic);
    return (char *)0;
  }

  uint32_t filename_size = get_name_size(file_header);
  uint32_t content_size = get_file_size(file_header);

  read_filename(addr, filename, filename_size);
  if (streq(filename, "TRAILER!!!") == 0) {
    return (char *)0;
  }

  if ((filename_size + sizeof(cpio_newc_header_t)) % 4 != 0) {
    filename_size += 4 - ((filename_size + sizeof(cpio_newc_header_t)) & 3);
  }

  if (content_size % 4 != 0) {
    content_size += 4 - (content_size & 3);
  }

  addrptr += sizeof(cpio_newc_header_t) + filename_size + content_size;
  return addrptr;
}

void cpio_ls() {
  char *addr = INITRD_ADDR;
  while (1) {
    char filename[2 * sizeof("TRAILER!!!")];
    addr = cpio_list_one(addr, filename);
    if (addr == (char *)0)
      break;
    printf("%s\n", filename);
  }
}

void cpio_cat(char *command, int len) {
  char *addr = INITRD_ADDR;
  char *new_addr;
  while (1) {
    char filename[2 * sizeof("TRAILER!!!")];
    new_addr = cpio_list_one(addr, filename);
    if (new_addr == (char *)0)
      break;

    if (strneq(command, filename, len) == 0) {
      cpio_newc_header_t *file_header = (cpio_newc_header_t *)addr;
      uint32_t filename_size = get_name_size(file_header);
      uint32_t content_size = get_file_size(file_header);

      if ((filename_size + sizeof(cpio_newc_header_t)) % 4 != 0) {
        filename_size += 4 - ((filename_size + sizeof(cpio_newc_header_t)) & 3);
      }
      if (content_size % 4 != 0) {
        content_size += 4 - (content_size & 3);
      }

      addr += sizeof(cpio_newc_header_t) + filename_size;
      for (int i = 0; i < content_size; i++) {
        printf("%c", addr[i]);
      }
      print("\n");
    }
    addr = new_addr;
  }
}

char *cpio_find_file(const char *file) {
  char *ptr = INITRD_ADDR;
  char *new_addr;
  char *program_addr = ptr;
  char filename[2 * sizeof("TRAILER!!!")];
  while (1) {
    new_addr = cpio_list_one(ptr, filename);
    if (new_addr == (char *)0)
      break;
    if (streq(file, filename) == 0) {
      cpio_newc_header_t *file_header = (cpio_newc_header_t *)ptr;
      uint32_t filename_size = get_name_size(file_header);

      if ((filename_size + sizeof(cpio_newc_header_t)) % 4 != 0) {
        filename_size += 4 - ((filename_size + sizeof(cpio_newc_header_t)) & 3);
      }

      program_addr = ptr + sizeof(cpio_newc_header_t) + filename_size;
      logdf("program_addr: %#X\n", program_addr);
      return program_addr;
    }
    ptr = new_addr;
  }
  return NULL;
}

int sys_exec(const char *file, char *const argv[]) {
  char *program_addr = cpio_find_file(file);

  if (program_addr == NULL) {
    printf("no such file or directory: %s\n", file);
    return -1;
  }

  thread_desc_t thread = get_cur_thread();
  struct trap_frame *tf = get_trap_frame(thread);

  // reset the general purpose registers
  memset(tf->x, 0, sizeof(tf->x));
  // reset the elr_el1
  tf->elr_el1 = (uint64_t)program_addr;
  return 0;
}
