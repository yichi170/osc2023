#ifndef _PROCESS_H
#define _PROCESS_H

#include "thread.h"

#define MAX_NUM_PROCS 64
#define USER_STACK_SIZE (4096 * 4)

struct trap_frame {
  uint64_t x[31]; // general registers from x0 - x30
  uint64_t spsr_el1;
  uint64_t elr_el1;
  uint64_t sp_el0;
};

typedef uint64_t pid_t;

void exit_process();
void kill_process(int);
pid_t sys_getpid();
void copy_trap_frame(thread_desc_t, thread_desc_t);
struct trap_frame *get_trap_frame(thread_desc_t);

#endif
