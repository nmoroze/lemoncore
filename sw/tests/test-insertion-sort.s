#########################################################
# RISC-V Insertion Sort
# Adapted from RISC-V Reader Figure 2.8
# a1 is n, a3 points to a[0], a4 is i, a5 is j, a6 is x
#########################################################

  addi a3,a0,4                    # a3 is pointer to a[i]
  addi a4,x0,1                    # i = 1
outer_loop:
  bltu a4,a1,continue_outer_loop  # if i < n, jump to Continue Outer loop
exit_outer_loop:
  jal x0,exit                     # return from function
continue_outer_loop:
  lw a6,0(a3)                     # x = a[i]
  addi a2,a3,0                    # a2 points to a[j]
  addi a5,a4,0                    # j = i
inner_loop:
  lw a7,-4(a2)                    # a7 = a[j-1]
  bge a6,a7,exit_inner_loop       # if a[j-1] <= a[i], jump to Exit Inner Loop
  sw a7,0(a2)                     # a[j] = a[j-1]
  addi a5,a5,-1                   # j--
  addi a2,a2,-4                   # decrement a2 to point to a[j]
  bne a5,x0,inner_loop            # if j != 0, jump to Inner Loop
exit_inner_loop:
  slli a5,a5,0x2                  # multiply a5 by 4
  add a5,a0,a5                    # a5 is now byte address of a[j]
  sw a6,0(a5)                     # a[j] = x
  addi a4,a4,1                    # i++
  addi a3,a3,4                    # increment a3 to point to a[i]
  jal x0,outer_loop               # jump to Outer Loop
exit:
  addi x31,x0,1                   # flag to signal done to TB
