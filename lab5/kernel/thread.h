#ifndef _THREAD_H
#define _THREAD_H

#include "type.h"

#define NULL (void *)0
#define MAX_NUM_THREADS 64
#define STACK_SIZE (4096 * 4)
#define STACK_ADDR (void *)(0x3C000000)

#define INIT_ATTR (thread_attr_t){ 0, NULL, 0 }
#define INIT_CONTEXT (struct context) \
{ { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, \
  0, 0, 0 }

struct callee_saved_regs {
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
};

struct context {
  struct callee_saved_regs cregs;
  uint64_t fp;
  uint64_t lr;
  uint64_t sp;
};

typedef enum {
  T_RUNNING = 0,
  T_READY,
  T_SUSPENDED,
  T_TERMINATED,
} thread_state_t;

typedef struct {
  int thread_id;
  void *stack_addr;
  uint64_t stacksize;
} thread_attr_t;

typedef struct {
  void (*start_routine)(void *);
  void *args;
} thread_start_args;

typedef struct thread_desc* thread_desc_t;
struct thread_desc {
  struct context ctx;
  thread_start_args start_args;
  thread_attr_t attr;
  thread_state_t state;
  thread_desc_t next;
};

typedef uint64_t thread_t;

void init_thread();
int thread_create(void (*)(void *), void *);
void thread_entry();
thread_desc_t get_cur_thread();
void thread_dump(thread_desc_t);
void context_dump(struct context *);
void demo_thread();

/* Ready Queue */
void init_ready();
void push_to_ready(thread_desc_t);
thread_desc_t pop_from_ready();

#endif
