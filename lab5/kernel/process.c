#include "process.h"
#include "print.h"
#include "string.h"
#include "mm_alloc.h"
#include "syscall_api.h"

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

  memcpy(dst_trap_frame, src_trap_frame, sizeof(struct trap_frame));
}

struct trap_frame *get_trap_frame(thread_desc_t thread) {
  return (struct trap_frame *)((uint64_t)thread->kstack + STACK_SIZE - sizeof(struct trap_frame));
}

void move_to_userspace(void *entry) {
  thread_desc_t thread = get_cur_thread();

  struct trap_frame *tf = get_trap_frame(thread);

  tf->spsr_el1 = 0x340;
  tf->elr_el1 = (uint64_t)entry;
  tf->sp_el0 = (uint64_t)thread->ustack + STACK_SIZE;

  asm volatile(
    "mov sp, %0\n\t" // move sp back to the trap frame address
    "msr sp_el0, %1\n\t"
    "bl back_to_user_space\n\t"
    :: "r" (tf), "r" (tf->sp_el0)
  );
}

static int shared = 1;
void test_syscall_in_usrspace() {
  printf("User thread id: %d\n", getpid());

  int pid = fork();
  if (pid == 0) {
    printf("Child says hello!\n");
    while(1) {
      printf("Please don't kill me :(\n");
      shared++;
    }
  } else if (pid > 0) {
    int r;
    printf("Parent says, \"My child has pid %d\"\n", pid);
    printf("Shared? %d\n", shared);
    r = 1000000;
    while (r--) { asm volatile("nop"); }
    printf("Kill my own child :(\n");
    kill(pid);
    r = 1000000;
    while (r--) { asm volatile("nop"); }
    printf("shared %d\n", shared);
  }

  exit(0);
}

void demo_syscall() {
  move_to_userspace(test_syscall_in_usrspace);
}
