#include "timer.h"

#include "malloc.h"
#include "print.h"

timer_event_t *time_events;

uint64_t get_timer() {
  uint64_t timer;
  __asm volatile(
    "mrs %0, cntpct_el0\n\t": "=r" (timer)
  );
  return timer;
}

uint64_t get_freq() {
  uint64_t freq;
  __asm volatile(
    "mrs %0, cntfrq_el0\n\t": "=r" (freq)
  );
  return freq;
}

void set_timeout(uint64_t timeout) {
  __asm volatile(
    "mov x1, %[time]\n\t"
    "msr cntp_tval_el0, x1\n\t"
    :: [time] "r" (timeout)
  );
}

void add_timer(void (*callback)(void *), void *data, uint64_t timeout) {

  timer_event_t *new_event = (timer_event_t *)simple_malloc(sizeof(timer_event_t));

  new_event->timeout = timeout * get_freq() + get_timer();
  new_event->event = callback;
  new_event->data = data;

  bool reset_next_timeout = false;

  if (time_events == (void *)0) {
    time_events = new_event;
    reset_next_timeout = true;
  } else if (time_events->timeout > new_event->timeout) {
    new_event->next = time_events;
    time_events = new_event;
    reset_next_timeout = true;
  } else {
    timer_event_t *prev = time_events, *next = time_events->next;
    while (next && prev->timeout < new_event->timeout) {
      prev = next;
      next = next->next;
    }
    prev->next = new_event;
    new_event->next = next;
  }

  if (reset_next_timeout) {
    set_timeout(new_event->timeout);
  }
}


void el1_timer_irq_handler() {
  if (time_events == (void *)0) {
    __asm volatile(
      "mrs x2, cntfrq_el0\n\t"
      "mov x1, #0x0000FFFF\n\t"
      "mul x2, x2, x1\n\t"
      "msr cntp_tval_el0, x2\n\t"
    );
    return;
  }

  time_events->event(time_events->data);
  time_events = time_events->next;
  
  uint64_t timeout = time_events->timeout;
  set_timeout(timeout);
}
