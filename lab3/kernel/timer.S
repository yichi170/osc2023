#define CORE0_TIMER_IRQ_CTRL 0x40000040

.globl core_timer_enable
core_timer_enable:
  mov x0, 1
  msr cntp_ctl_el0, x0 // enable the counter and interrupt generation
  mrs x0, cntfrq_el0
  msr cntp_tval_el0, x0 // set expired time
  mov x0, 2
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret

.globl core_timer_irq_handler
core_timer_irq_handler:
  mrs x2, cntfrq_el0
  add x2, x2, x2
  msr cntp_tval_el0, x2
  bl  print_timer_irq // void print_timer_irq(uint64_t frq_timer, uint64_t cnt_timer)
  ret

.globl core_timer_disable
core_timer_disable:
  mov x0, 0
  msr cntp_ctl_el0, x0 // disable
  mov x0, 0
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret