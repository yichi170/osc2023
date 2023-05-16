#include "syscall.h"
#include "thread.h"
#include "mini_uart.h"
#include "fork.h"
#include "mailbox.h"
#include "print.h"

uint32_t sys_uart_recv(char buf[], uint32_t size) {
  int i;
  char ch;
  for (i = 0; i < size; i++) {
    ch = uart_recv();
    buf[i] = ch;
    if (ch == '\0') break;
  }
  return i;
}

uint32_t sys_uart_send(const char buf[], uint32_t size) {
  int i;
  for (i = 0; i < size; i++) {
    if (buf[i] == '\0') break;
    putc(buf[i]);
  }
  return i;
}

int sys_exec(const char *name, char *const argv[]) {}

void sys_exit() {
  exit_process();
}

int sys_mbox_call(unsigned char channel, volatile unsigned int *mbox) {
  return mailbox_call(channel, mbox);
}

void sys_kill(int pid) {
  kill_process(pid);
}

const void *const syscall_table[] = {
  sys_getpid,
  sys_uart_recv,
  sys_uart_send,
  sys_exec,
  sys_fork,
  sys_exit,
  sys_mbox_call, // need to modify the interface
  sys_kill
};
