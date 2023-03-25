#include "mini_uart.h"
#include "devtree.h"
#include "cpio.h"
#include "shell.h"

extern void set_exception_vector_table();

void kernel_main() {
  set_exception_vector_table();
  uart_init();
  fdt_traverse(initramfs_callback);
  shell();
}
