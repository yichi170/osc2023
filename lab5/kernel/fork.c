#include "fork.h"
#include "thread.h"
#include "string.h"

#define NULL (void *)0

int sys_fork() {
  thread_desc_t parent_thread = get_cur_thread();
  thread_t child_tid = thread_create(NULL, NULL);
  thread_desc_t child_thread = get_thread(child_tid);

  // copy the stack
  memcpy(child_thread->stack_addr, parent_thread->stack_addr, STACK_SIZE);

  return child_tid;
}