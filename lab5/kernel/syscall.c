#include "syscall.h"
#include "thread.h"
#include "mini_uart.h"
#include "fork.h"
#include "mailbox.h"
#include "print.h"

int exec(const char *name, char *const argv[]) {}

void exit() {}

void kill(int pid) {}

const void *const syscall_table[] = {
  getpid,
  uart_recv,
  uart_send,
  exec,
  fork,
  exit,
  mailbox_call, // need to modify the interface
  kill
};
