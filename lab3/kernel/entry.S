// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 8
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm

invalid_exception_handler:
  msr DAIFSet, 0xf // disable interrupt
  save_all
  mrs x0, spsr_el1
  mrs x1, elr_el1
  mrs x2, esr_el1
  mov x3, 1
  bl  print_exception
  load_all
  msr DAIFClr, 0xf // enable interrupt
  eret

lower_el_sync_exception:
  msr DAIFSet, 0xf // disable interrupt
  save_all
  mrs x0, spsr_el1
  mrs x1, elr_el1
  mrs x2, esr_el1
  mov x3, 0
  bl  print_exception
  load_all
  msr DAIFClr, 0xf // enable interrupt
  eret

el1_irq_exception_handler:
  save_all
  bl c_el1_irq_handler
  load_all
  eret

irq_exception_handler:
  msr DAIFSet, 0xf // disable interrupt
  save_all
  bl c_el0_irq_handler
  load_all
  msr DAIFClr, 0xf // enable interrupt
  eret

.align 11 // vector table should be aligned to 0x800
.globl exception_vector_table
exception_vector_table:
  b invalid_exception_handler // branch to a handler function. // sync, Current EL with SP0
  .align 7 // entry size is 0x80, .align will pad 0
  b invalid_exception_handler // irq
  .align 7
  b invalid_exception_handler // fiq
  .align 7
  b invalid_exception_handler // serr
  .align 7

  b invalid_exception_handler // sync, Current EL with SPx
  .align 7
  b el1_irq_exception_handler // irq
  .align 7
  b invalid_exception_handler // fiq
  .align 7
  b invalid_exception_handler // serr
  .align 7

  b lower_el_sync_exception // sync, Lower EL using AArch64
  .align 7
  b irq_exception_handler // irq
  .align 7
  b invalid_exception_handler // fiq
  .align 7
  b invalid_exception_handler // serr
  .align 7

  b invalid_exception_handler // sync, Lower EL using AArch32
  .align 7
  b invalid_exception_handler // irq
  .align 7
  b invalid_exception_handler // fiq
  .align 7
  b invalid_exception_handler // serr
  .align 7

.globl set_exception_vector_table
set_exception_vector_table:
  adr x0, exception_vector_table
  msr vbar_el1, x0
  ret