jal x0, main
exception:
  # increment x1 each time we get to the handler
  addi x1, x1, 1
  csrr x10, mepc
  addi x10, x10, 4
  csrw mepc, x10
  mret
main:
  csrw mtvec, 4 # set mtvec to exception handler
  li x2, 25
ecall:
  ecall
  blt x1, x2, ecall
  li x31, 1
