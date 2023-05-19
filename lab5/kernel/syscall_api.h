#ifndef _SYSCALL_API_H
#define _SYSCALL_API_H

#define SYS_ID_GETPID 0
#define SYS_ID_UART_READ 1
#define SYS_ID_UART_WRITE 2
#define SYS_ID_EXEC 3
#define SYS_ID_FORK 4
#define SYS_ID_EXIT 5
#define SYS_ID_MBOX_CALL 6
#define SYS_ID_KILL 7

#ifndef __ASSEMBLER__

#include "type.h"

int getpid();
uint32_t uartread(char [], uint32_t);
uint32_t uartwrite(const char [], uint32_t);
int exec(const char *, char *const []);
int fork();
void exit(int);
int mbox_call(unsigned char, volatile unsigned int *);
void kill(int);

#endif
#endif
