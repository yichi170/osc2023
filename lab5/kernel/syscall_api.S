#include "syscall_api.h"

.globl getpid
getpid:
  mov x8, #SYS_ID_GETPID
  svc #0
  ret

.globl uart_read
uart_read:
  mov x8, #SYS_ID_UART_READ
  svc #0
  ret

.globl uart_write
uart_write:
  mov x8, #SYS_ID_UART_WRITE
  svc #0
  ret

.globl exec
exec:
  mov x8, #SYS_ID_EXEC
  svc #0
  ret

.globl fork
fork:
  mov x8, #SYS_ID_FORK
  svc #0
  ret

.globl exit
exit:
  mov x8, #SYS_ID_EXIT
  svc #0
  ret

.globl mbox_call
mbox_call:
  mov x8, #SYS_ID_MBOX_CALL
  svc #0
  ret

.globl kill
kill:
  mov x8, #SYS_ID_KILL
  svc #0
  ret