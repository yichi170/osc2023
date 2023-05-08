.globl from_el2_to_el1
from_el2_to_el1:
    mov     x0, (1 << 31)
    msr     hcr_el2, x0
    mov     x0, 0x345
    // 0x3c5 = 0000 0000 0000 0000 0000 0011 1100 0101
    // unmask bit[7] (enable IRQ interrupt)
    // 0x345 = 0000 0000 0000 0000 0000 0011 0100 0101
    // PSTATE ARMv8 (PSRs ARMv7): https://developer.arm.com/documentation/den0024/a/ARMv8-Registers/Processor-state
    msr     spsr_el2, x0
    msr     elr_el2, lr
    eret

.globl from_el1_to_el0
from_el1_to_el0:
  mov x2, 0x340
  msr spsr_el1, x2
  msr elr_el1, x0
  msr sp_el0, x1
  eret