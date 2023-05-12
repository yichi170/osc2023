.globl from_el2_to_el1
from_el2_to_el1:
    mov     x0, (1 << 31)
    msr     hcr_el2, x0
    mov     x0, 0x345
    // 0x3c5 = 0000 0000 0000 0000 0000 0011 1100 0101
    // 0x345 = 0000 0000 0000 0000 0000 0011 0100 0101
    // I            bits[7]            IRQ
    // F            bits[6]            FIQ
    // M[3:0]       bits[3:0]          Exception level and selected Stack Pointer.
    // PSTATE ARMv8 (PSRs ARMv7): https://developer.arm.com/documentation/den0024/a/ARMv8-Registers/Processor-state
    // spsr_el1: https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/SPSR-EL1--Saved-Program-Status-Register--EL1-
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