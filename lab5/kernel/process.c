#include "process.h"
#include "print.h"
#include "string.h"
#include "mm_alloc.h"

void exit_process() {
  thread_desc_t cur_thread = get_cur_thread();
  cur_thread->state = T_TERMINATED;

  while (1) { asm volatile("nop"); }
}

void kill_process(int pid) {
  if (pid == get_cur_thread()->thread_id) {
    exit_process();
  } else {
    thread_desc_t thread = get_thread(pid);
    thread->state = T_TERMINATED;
    remove_from_ready(pid);
  }
}

pid_t sys_getpid() {
  return get_cur_thread()->thread_id;
}

void copy_trap_frame(thread_desc_t dst_thread, thread_desc_t src_thread) {
  struct trap_frame *src_trap_frame = get_trap_frame(src_thread);
  struct trap_frame *dst_trap_frame = get_trap_frame(dst_thread);

  memcpy(src_trap_frame, dst_trap_frame, sizeof(struct trap_frame));
}

struct trap_frame *get_trap_frame(thread_desc_t thread) {
  return (struct trap_frame *)(thread->kstack + STACK_SIZE - sizeof(struct trap_frame));
}
