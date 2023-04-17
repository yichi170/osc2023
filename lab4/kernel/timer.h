#ifndef _TIMER_H
#define _TIMER_H

#include "type.h"

struct timer_event {
  void (*event)(void *data);
  void *data;
  uint64_t timeout;
  struct timer_event *next;
};

typedef struct timer_event timer_event_t;

void add_timer(void (*)(void *), void *, uint64_t);
void el1_timer_irq_handler();
void print_timer_irq();

#endif
