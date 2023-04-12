#include "mini_uart.h"
#include "devtree.h"
#include "cpio.h"
#include "shell.h"

extern void set_exception_vector_table();
extern void core_timer_enable();

void kernel_main() {
  set_exception_vector_table();
  uart_init();
  fdt_traverse(initramfs_callback);
  core_timer_enable();
  shell();
}
