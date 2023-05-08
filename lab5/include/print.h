#ifndef _PRINT_H
#define _PRINT_H

#include "type.h"

#ifndef NDEBUG
  #define logd(x) print(x)
  #define logdf(x, ...) printf(x, __VA_ARGS__)
#else
  #define logd(x)
  #define logdf(x, ...)
#endif

void putc(char);
void print(char *);
void printhex(unsigned int, bool);
void printf(char *str, ...);
void uart_send_int(int);

#endif
