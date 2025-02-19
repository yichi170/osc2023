#include "fork.h"
#include "thread.h"
#include "string.h"
#include "print.h"
#include "exception.h"

void child_process_entry() {
  struct trap_frame *cur_trapframe = get_trap_frame(get_cur_thread());
  cur_trapframe->x[0] = 0;

  printf("[child_process] pid: %d @child_process_entry\n", get_cur_thread()->thread_id);

  // move to user space
  asm volatile(
    "mov sp, %0\n\t" // move sp back to the trap frame address
    "msr sp_el0, %1\n\t"
    "bl back_to_user_space\n\t"
    ::
    "r" ((uint64_t)cur_trapframe),
    "r" (cur_trapframe->sp_el0)
  );
}

int sys_fork() {
  disable_interrupt();
  thread_desc_t parent_thread = get_cur_thread();
  thread_t child_tid = thread_create(child_process_entry, NULL);
  thread_desc_t child_thread = get_thread(child_tid);

  uint64_t elr_el1;
  asm volatile("mrs %0, elr_el1\n\t": "=r" (elr_el1));

  // copy the user stack
  memcpy(child_thread->ustack, parent_thread->ustack, STACK_SIZE);
  // copy the trap frame
  copy_trap_frame(child_thread, parent_thread);

  // set the child process for running in el0
  struct trap_frame *child_trapframe = get_trap_frame(child_thread);
  child_trapframe->spsr_el1 = 0x340;
  child_trapframe->elr_el1 = elr_el1;
  child_trapframe->sp_el0 = (uint64_t)child_thread->ustack + STACK_SIZE;

  enable_interrupt();
  return child_tid;
}