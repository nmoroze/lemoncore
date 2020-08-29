# Main program entry point for all lemonsoc programs
.global _entry
_entry:
    # set up scratch space for saving regs on exception handler
    la x1, _mscratch
    csrw mscratch, x1
    # set up exception handler
    la x1, _handler
    csrw mtvec, x1

    # set stack pointer and clear all registers
    li ra, 0
    la sp, _stack_start
    li x3, 0
    li x4, 0
    li x5, 0
    li x6, 0
    li x7, 0
    li x7, 0
    li x8, 0
    li x9, 0
    li x10, 0
    li x11, 0
    li x12, 0
    li x13, 0
    li x14, 0
    li x15, 0
    li x16, 0
    li x17, 0
    li x18, 0
    li x19, 0
    li x20, 0
    li x21, 0
    li x22, 0
    li x23, 0
    li x24, 0
    li x25, 0
    li x26, 0
    li x27, 0
    li x28, 0
    li x29, 0
    li x30, 0
    li x31, 0

    # enable timer interrupts
    li x1, 128 # 1 << 7
    csrw mie, x1
    csrwi mstatus, 8 # 1 << 3

    # call main function
    call main

    # light done LED, in case we happen to return
    li x1, 0x3000
    li x2, 0x1
    sw x2, 4(x1)

    # hang forever
    jal x0, _hang

# Exception handler stub
.global _handler
_handler:
    # save registers that we clobber
    csrrw a0, mscratch, a0
    sw a1, 0(a0)
    sw a2, 4(a0)

    # decode interrupt cause and jump to _exception if not timer irq
    csrr a1, mcause
    bgez a1, _exception
    andi a1, a1, 0x3f
    li a2, 7
    bne a1, a2, _exception

   # increment timer static variable
   la a1, timer
   lw a2, 0(a1)
   addi a2, a2, 1
   sw a2, 0(a1)

   # clear timer
   li a1, 0x300C
   sw x0, 0(a1)

   # restore registers
   lw a2, 4(a0)
   lw a1, 0(a0)
   csrrw a0, mscratch, a0
   mret

_exception:
    # light exception LED
    li x1, 0x3000
    li x2, 0x2
    sw x2, 4(x1)

    # hang forever
    jal x0, _hang

# Hang stub
_hang:
  jal x0, _hang

# Scratch space for saving registers during exception handler
.data
_mscratch: .space 8
