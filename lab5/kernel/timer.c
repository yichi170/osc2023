#include "timer.h"
#include "sched.h"
#include "malloc.h"
#include "print.h"

timer_event_t *time_events;

uint64_t get_timer() {
  volatile uint64_t timer;
  __asm volatile(
    "mrs %0, cntpct_el0\n\t": "=r" (timer)
  );
  return timer;
}

uint64_t get_freq() {
  volatile uint64_t freq;
  __asm volatile(
    "mrs %0, cntfrq_el0\n\t": "=r" (freq)
  );
  return freq;
}

void set_timeout(uint64_t timeout) {
  __asm volatile(
    "msr cntp_cval_el0, %0\n\t"
    :: "r" (timeout)
  );
}

void add_timer(void (*callback)(void *), void *data, uint64_t timeout) {

  timer_event_t *new_event = (timer_event_t *)simple_malloc(sizeof(timer_event_t));

  new_event->timeout = timeout * get_freq();
  new_event->event = callback;
  new_event->data = data;
  new_event->next = (void *)0;

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

  if (reset_next_timeout)
    set_timeout(new_event->timeout);
}

void log_timer_irq() {
  uint64_t freq = get_freq();
  uint64_t cnt = get_timer();
  logd("===== timer IRQ info =====\n");
  logdf("Current time: %#X\n", cnt / freq);
}


void el1_timer_irq_handler() {
  round_robin_scheduler();

  if (time_events == (void *)0) {
    uint64_t time;
    __asm volatile(
      "mrs x2, cntfrq_el0\n\t"
      "add x2, x2, x2\n\t"
      "msr cntp_tval_el0, x2\n\t"
      "mrs %0, cntpct_el0\n\t": "=r" (time)
    );
    log_timer_irq();
    return;
  }

  time_events->event(time_events->data);
  time_events = time_events->next;
  
  uint64_t timeout = time_events->timeout;
  set_timeout(timeout);
}
