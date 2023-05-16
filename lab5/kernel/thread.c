#include "thread.h"
#include "malloc.h"
#include "mm_alloc.h"
#include "print.h"
#include "sched.h"

static int thread_counter = 0;
thread_desc_t initial_thread = NULL;
static thread_desc_t ready_queue_head = NULL;
static thread_desc_t ready_queue_tail = NULL;
static thread_desc_t threads[MAX_NUM_THREADS];

extern struct context *get_context();

void init_thread() {
  printf("=========== Init Thread ===========\n");
  init_ready();

  for (int i = 0; i < MAX_NUM_THREADS; i++)
    threads[i] = NULL;

  initial_thread = kmalloc(sizeof(struct thread_desc));
  initial_thread->start_args = (thread_start_args){ NULL };
  initial_thread->thread_id = 0;
  initial_thread->state = T_RUNNING;
  initial_thread->next = NULL;
  initial_thread->kstack = kmalloc(STACK_SIZE);
  asm volatile("msr tpidr_el1, %0" : : "r"(initial_thread));

  threads[0] = initial_thread;
  ++thread_counter;
}

int thread_create(void (*func)(void *), void *args) {
  thread_desc_t new_thread = kmalloc(sizeof(struct thread_desc));

  thread_t t_id = thread_counter++;
  void *stack_addr = kmalloc(STACK_SIZE);
  void *user_stack_addr = kmalloc(STACK_SIZE);

  new_thread->ctx = INIT_CONTEXT;
  new_thread->start_args = (thread_start_args){ func, args };
  new_thread->thread_id = t_id;
  new_thread->state = T_READY;
  new_thread->next = NULL;
  new_thread->kstack = stack_addr;
  new_thread->ustack = user_stack_addr;

  new_thread->ctx.cregs.x28 = t_id;
  new_thread->ctx.sp = (uint64_t)(stack_addr + STACK_SIZE);
  new_thread->ctx.lr = (uint64_t)thread_entry;

  threads[t_id % MAX_NUM_THREADS] = new_thread;
  push_to_ready(new_thread);

  return t_id;
}

void thread_entry() {
  thread_desc_t ct = get_cur_thread();
  thread_start_args *start_args = &ct->start_args;
  start_args->start_routine(start_args->args);
  // thread_dump(ct);
  ct->state = T_TERMINATED;
  schedule();
  while (1) {}
}

thread_desc_t get_cur_thread() {
  struct context *cur_context = get_context();
  thread_desc_t cur_thread = (thread_desc_t)cur_context;
  return cur_thread;
}

thread_desc_t get_thread(int t_id) {
  return threads[t_id];
}

void kill_zombies() {
  for (int i = 0; i < MAX_NUM_THREADS; i++) {
    if (threads[i]->state == T_TERMINATED) {
      kfree((void *)threads[i]->ctx.sp);
      kfree(threads[i]);
      threads[i] = NULL;
    }
  }
}

void thread_dump(thread_desc_t thread) {
  if (thread != NULL) {
    print("thread info:\n");
    printf("- id: %d\n", thread->thread_id);
    printf("- thread_func: %#X\n", thread->start_args.start_routine);
  }
}

void context_dump(struct context *ctx) {
  printf("context info:\n");
  printf("- x19: %#X\t", ctx->cregs.x19);
  printf("  x20: %#X\n", ctx->cregs.x20);
  printf("- x21: %#X\t", ctx->cregs.x21);
  printf("  x22: %#X\n", ctx->cregs.x22);
  printf("- x23: %#X\t", ctx->cregs.x23);
  printf("  x24: %#X\n", ctx->cregs.x24);
  printf("- x25: %#X\t", ctx->cregs.x25);
  printf("  x26: %#X\n", ctx->cregs.x26);
  printf("- x27: %#X\t", ctx->cregs.x27);
  printf("  x28: %#X\n", ctx->cregs.x28);

  printf("-  fp: %#X\n", ctx->fp);
  printf("-  lr: %#X\n", ctx->lr);
  printf("-  sp: %#X\n", ctx->sp);
}

void idle() {
  while (1) {
    kill_zombies();
    schedule();
  }
}

/* ------------------------------------ *
 *                 DEMO                 *
 * ------------------------------------ */

void foo() {
  thread_desc_t cur_thread = get_cur_thread();
  int r;

  for (int i = 0; i < 10; i++) {
    printf("thread #%d: print %d\n", cur_thread->thread_id, i);
    r = 1000000;
    while (r--) { asm volatile("nop"); }
    schedule();
  }
}

void demo_thread() {
  for (int i = 0; i < 2; i++) {
    thread_create(foo, NULL);
  }
  idle();
}

/* ------------------------------------ *
 *             READY QUEUE              *
 * ------------------------------------ */

void init_ready() {
  ready_queue_head = NULL;
  ready_queue_tail = NULL;
}

void push_to_ready(thread_desc_t n_thread) {
  n_thread->next = NULL;
  if (ready_queue_head != NULL) {
    ready_queue_tail->next = n_thread;
    ready_queue_tail = ready_queue_tail->next;
  } else {
    ready_queue_head = n_thread;
    ready_queue_tail = ready_queue_head;
  }
  printf("push 'thread #%d' into queue\n", n_thread->thread_id);
}

thread_desc_t pop_from_ready() {
  if (ready_queue_head == ready_queue_tail && ready_queue_head == NULL)
    return NULL;

  thread_desc_t top = ready_queue_head;
  ready_queue_head = ready_queue_head->next;
  if (ready_queue_head == NULL) // [ready_queue_head, ready_queue_tail]
    ready_queue_tail = NULL;
  top->next = NULL;
  printf("pop 'thread #%d' from queue\n", top->thread_id);
  return top;
}

void remove_from_ready(int tid) {
  if (ready_queue_head == NULL) {
    printf("failed to kill thread #%d\n", tid);
    return;
  }

  if (ready_queue_head->thread_id == tid) {
    ready_queue_head = ready_queue_head->next;
    if (ready_queue_head == NULL)
      ready_queue_tail = NULL;
    return;
  }

  thread_desc_t ptr_prev = ready_queue_head;
  thread_desc_t ptr = ready_queue_head->next;

  while (ptr != NULL) {
    if (ptr->thread_id == tid) {
      ptr_prev->next = ptr->next;
      if (ptr_prev->next == NULL)
        ready_queue_tail = ptr_prev;
      return;
    }
    ptr = ptr->next;
    ptr_prev = ptr_prev->next;
  }
  printf("failed to kill thread #%d\n", tid);
}
