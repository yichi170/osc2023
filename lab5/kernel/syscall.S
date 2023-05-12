.extern syscall_table

.globl syscall_entry
syscall_entry:
  adr    x19, syscall_table
  lsl    x20, x8, #3    // multiply x8 by 8 to get the offset in bytes
  add    x19, x19, x20
  ldr    x20, [x19]
  blr    x20            // branch to target syscall function
  str    x0, [sp]
  ret
