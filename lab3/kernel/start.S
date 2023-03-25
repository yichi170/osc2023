.section ".data"

.globl DTB_ADDR

.align 4
DTB_ADDR:
    .quad 0

.section ".text.boot"

.globl _start

_start:
_set_dtb_addr:
    ldr     x1, =DTB_ADDR
    str     x0, [x1]

_setup:
    bl      from_el2_to_el1
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, _one

_loop:
    wfe
    b       _loop

_one:
    ldr     x0, =_start
    mov     sp, x0

    adr     x0, __bss_begin
    adr     x1, __bss_end
    sub     x1, x1, x0
_clean_bss:
    cbz     x1, _kernel
    str     xzr, [x0], #8
    subs    x1, x1, #8
    cbnz    x1, _clean_bss

_kernel:
    bl      kernel_main
    b       _loop

from_el2_to_el1:
    mov     x0, (1 << 31)
    msr     hcr_el2, x0
    mov     x0, 0x3c5
    // 0x3c5 = 0000 0000 0000 0000 0000 0011 1100 0101
    msr     spsr_el2, x0
    msr     elr_el2, lr
    eret
