#include "sched.h"
#include "print.h"

extern void switch_context(struct context *, struct context *);

void schedule() {
  thread_desc_t cur_thread = get_cur_thread();
  thread_desc_t next_thread = pop_from_ready();

  if (cur_thread->state == T_RUNNING) {
    cur_thread->state = T_READY;
    if (cur_thread->thread_id != 0)
      push_to_ready(cur_thread);
  }

  if (next_thread != NULL) {
    struct context *next_context = &next_thread->ctx;
    next_thread->state = T_RUNNING;
    switch_context(&cur_thread->ctx, &next_thread->ctx);
  } else {
    kill_zombies();
    if (cur_thread->thread_id == 0)
      return;
    switch_context(&cur_thread->ctx, &initial_thread->ctx);
  }
}
