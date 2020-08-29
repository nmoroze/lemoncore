#include "util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void print_instruction(uint32_t instr) {
  int rd = (instr >> 7) & 0x1F;
  int rs1 = (instr >> 15) & 0x1F;
  int rs2 = (instr >> 20) & 0x1F;
  int funct = (instr >> 12) & 0x7;
  int imm;
  printf("0x%08x ", instr);

  switch (instr & 0x7F) {
  case 0b0110111:
    printf("lui r%d, %x\n",  rd, instr >> 12);
    break;
  case 0b0010111:
    printf("auipc r%d, %x\n", rd, instr >> 12);
    break;
  case 0b1101111:
    // TODO: reswizzle imm
    printf("jal r%d, ...\n", rd);
    break;
  case 0b1100111:
    printf("jalr r%d, %x\n", rd, instr >> 12);
    break;
  case 0b1100011:
    switch (funct) {
    case 0b000:
      printf("beq r%d, r%d\n", rs1, rs2);
      break;
    case 0b001:
      printf("bne r%d, r%d\n", rs1, rs2);
      break;
    case 0b100:
      printf("blt r%d, r%d\n", rs1, rs2);
      break;
    case 0b101:
      printf("bge r%d, r%d\n", rs1, rs2);
      break;
    case 0b110:
      printf("bltu r%d, r%d\n", rs1, rs2);
      break;
    case 0b111:
      printf("bgeu r%d, r%d\n", rs1, rs2);
      break;
    default: printf("Illegal branch instruction - funct: %d!\n", funct);
    }
    break;
  case 0b0000011:
    imm = (instr >> 20);
    switch (funct) {
    case 0b000:
      printf("lb r%d, %d(r%d)\n", rd, imm, rs1);
      break;
    case 0b001:
      printf("lh r%d, %d(r%d)\n", rd, imm, rs1);
      break;
    case 0b010:
      printf("lw r%d, %d(r%d)\n", rd, imm, rs1);
      break;
    case 0b100:
      printf("lbu r%d, %d(r%d)\n", rd, imm, rs1);
      break;
    case 0b101:
      printf("lhu r%d, %d(r%d)\n", rd, imm, rs1);
      break;
    default: printf("Illegal instruction: load!\n");
    }
    break;
  case 0b0100011:
    imm = ((instr >> 25) & 0x7F) | ((instr >> 7) & 0x1F);
    switch (funct) {
    case 0b000:
      printf("sb r%d, %d(r%d)\n", rs2, imm, rs1);
      break;
    case 0b001:
      printf("sh r%d, %d(r%d)\n", rs2, imm, rs1);
      break;
    case 0b010:
      printf("sw r%d, %d(r%d)\n", rs2, imm, rs1);
      break;
    default: printf("Illegal instruction: store!\n");
    }
    break;
  case 0b0010011:
    imm = (instr >> 20);
    switch (funct) {
    case 0b000:
      printf("addi r%d, r%d, %d\n", rd, rs1, imm);
      break;
    case 0b010:
      printf("slti r%d, r%d, %d\n", rd, rs1, imm);
      break;
    case 0b011:
      printf("sltiu r%d, r%d, %d\n", rd, rs1, imm);
      break;
    case 0b100:
      printf("xori r%d, r%d, %d\n", rd, rs1, imm);
      break;
    case 0b110:
      printf("ori r%d, r%d, %d\n", rd, rs1, imm);
      break;
    case 0b111:
      printf("andi r%d, r%d, %d\n", rd, rs1, imm);
      break;
    case 0b001:
      printf("slli r%d, r%d, %d\n", rd, rs1, imm);
      break;
    case 0b101:
      if (!(instr >> 30)) {
        printf("srli r%d, r%d, %d\n", rd, rs1, imm);
      } else {
        printf("srai r%d, r%d, %d\n", rd, rs1, imm);
      }
      break;
    default: printf("Illegal instruction: alui!\n");
    }
    break;
  case 0b0110011:
    switch (funct) {
    case 0b000:
      if (!(instr >> 30))
        printf("add r%d, r%d, r%d\n", rd, rs1, rs2);
      else
        printf("sub r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    case 0b001:
      printf("sll r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    case 0b010:
      printf("slt r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    case 0b011:
      printf("sltu r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    case 0b100:
      printf("xor r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    case 0b101:
      if (!(instr >> 30))
        printf("srl r%d, r%d, r%d\n", rd, rs1, rs2);
      else
        printf("sra r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    case 0b110:
      printf("or r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    case 0b111:
      printf("and r%d, r%d, r%d\n", rd, rs1, rs2);
      break;
    default: printf("Illegal instruction: alu!\n");
    }
    break;
  case 0b1110011:
    switch (funct) {
      case 0b000:
        if (instr >> 20)
          printf("ebreak\n");
        else
          printf("ecall\n");
        break;
      // TODO: output csr instruction operands
      case 0b001:
        printf("csrrw\n");
        break;
      case 0b010:
        printf("cssrs\n");
        break;
      case 0b011:
        printf("csrrc\n");
        break;
      case 0b101:
        printf("csrrwi\n");
        break;
      case 0b110:
        printf("csrrsi\n");
        break;
      case 0b111:
        printf("csrrci\n");
        break;
      default: printf("Illegal instruction: system!\n");
    }
    break;
  default: printf("Illegal instruction: invalid opcode!\n");
  }
}
