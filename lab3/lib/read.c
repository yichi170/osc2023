#include "read.h"
#include "type.h"
#include "print.h"
#include "mini_uart.h"

int readline(char *buf, int len, bool interact) {
  char ch;
  int i = 0;
  char *ptr = buf;
  *ptr = '\0';

  for (i = 0; i < len - 1; i++) {
    ch = uart_recv();

    if (ch == '\n' || ch == '\r') {
      if (interact == true)
        putc(ch);
      break;
    } else if (ch == 0x7f) { // 127
      --i; --ptr;
      if (ptr < buf)
        ptr = buf;
      putc(0x8); putc(' '); putc(0x8);
      continue;
    } else if (ch < 31 || ch > 127) {
      --i;
      continue;
    }
    if (interact == true)
      putc(ch);
    *ptr++ = ch;
  }
  *ptr = '\0';
  return i;
}
