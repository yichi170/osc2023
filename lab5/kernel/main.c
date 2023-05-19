#include "mini_uart.h"
#include "devtree.h"
#include "cpio.h"
#include "page_alloc.h"
#include "mm_alloc.h"
#include "mm_utils.h"
#include "thread.h"
#include "shell.h"
#include "sched.h"

extern void set_exception_vector_table();
extern void core_timer_enable();

extern char __heap_start;
static char *kernel_end_addr = &__heap_start;

void kernel_main() {
  set_exception_vector_table();
  uart_init();

  fdt_traverse(get_ramfs_addr);
  reserve_ramfs();
  core_timer_enable();

  init_allocator();
  init_pools();

  memory_reserve((void *)0x0, (void *)0x1000); // Spin tables for multicore boot
  memory_reserve((void *)0x80000, (void *)kernel_end_addr); // kernel image
  show_reserve();
  mark_reserve_memory();

  init_thread();
  start_initial_thread();
}
