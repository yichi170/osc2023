#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "mmio.h"
#include "mini_uart.h"
#include "type.h"

#define GPU_PENDING1    ((volatile unsigned int*)(MMIO_BASE+0x0000b204))

void async_print(const char *);
unsigned int async_read(char *, unsigned int);

void c_el1_irq_handler();
void print_exception(uint64_t);

void enable_interrupt();
void disable_interrupt();

#endif
