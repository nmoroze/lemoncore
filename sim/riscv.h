#ifndef RISCV_H
#define RISCV_H

#include <stdint.h>

#define RV_CSR_MISA		0x301
#define RV_CSR_MSCRATCH		0x340
#define RV_CSR_CYCLE 0xC00
#define RV_CSR_INSTRET 0xC02
#define RV_CSR_CYCLEH 0xC80
#define RV_CSR_INSTRETH 0xC82

uint32_t rv_lui(uint8_t rd, int32_t imm);
uint32_t rv_lui();
uint32_t rv_auipc(uint8_t rd, int32_t imm);
uint32_t rv_auipc();
uint32_t rv_jal(uint32_t rd, int32_t imm);
uint32_t rv_jal();
uint32_t rv_jalr(uint32_t rd, uint32_t rs1, int32_t imm);
uint32_t rv_jalr();
uint32_t rv_beq(uint8_t rs1, uint8_t rs2, int32_t imm);
uint32_t rv_beq();
uint32_t rv_bne(uint8_t rs1, uint8_t rs2, int32_t imm);
uint32_t rv_bne();
uint32_t rv_blt(uint8_t rs1, uint8_t rs2, int32_t imm);
uint32_t rv_blt();
uint32_t rv_bge(uint8_t rs1, uint8_t rs2, int32_t imm);
uint32_t rv_bge();
uint32_t rv_bltu(uint8_t rs1, uint8_t rs2, int32_t imm);
uint32_t rv_bltu();
uint32_t rv_bgeu(uint8_t rs1, uint8_t rs2, int32_t imm);
uint32_t rv_bgeu();
uint32_t rv_lb(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_lb();
uint32_t rv_lh(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_lh();
uint32_t rv_lw(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_lw();
uint32_t rv_lbu(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_lbu();
uint32_t rv_lhu(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_lhu();
uint32_t rv_sb(uint8_t rs2, uint8_t rs1, int32_t imm);
uint32_t rv_sb();
uint32_t rv_sh(uint8_t rs2, uint8_t rs1, int32_t imm);
uint32_t rv_sh();
uint32_t rv_sw(uint8_t rs2, uint8_t rs1, int32_t imm);
uint32_t rv_sw();
uint32_t rv_addi(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_addi();
uint32_t rv_slti(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_slti();
uint32_t rv_sltiu(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_sltiu();
uint32_t rv_xori(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_xori();
uint32_t rv_ori(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_ori();
uint32_t rv_andi(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_andi();
uint32_t rv_slli(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_slli();
uint32_t rv_srli(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_srli();
uint32_t rv_srai(uint8_t rd, uint8_t rs1, int32_t imm);
uint32_t rv_srai();
uint32_t rv_add(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_add();
uint32_t rv_sub(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_sub();
uint32_t rv_sll(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_sll();
uint32_t rv_slt(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_slt();
uint32_t rv_sltu(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_sltu();
uint32_t rv_xor(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_xor();
uint32_t rv_srl(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_srl();
uint32_t rv_sra(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_sra();
uint32_t rv_or(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_or();
uint32_t rv_and(uint8_t rd, uint8_t rs1, uint8_t rs2);
uint32_t rv_and();
uint32_t rv_fence(uint8_t pred, uint8_t succ);
uint32_t rv_fence();
uint32_t rv_fence_i();
uint32_t rv_ecall();
uint32_t rv_ebreak();
uint32_t rv_csrrw(uint8_t rd, uint8_t rs1, uint32_t csr_num);
uint32_t rv_csrrw();
uint32_t rv_csrrs(uint8_t rd, uint8_t rs1, uint32_t csr_num);
uint32_t rv_csrrs();
uint32_t rv_csrrc(uint8_t rd, uint8_t rs1, uint32_t csr_num);
uint32_t rv_csrrc();
uint32_t rv_csrrwi(uint8_t rd, uint8_t zimm, uint32_t csr_num);
uint32_t rv_csrrwi();
uint32_t rv_csrrsi(uint8_t rd, uint8_t zimm, uint32_t csr_num);
uint32_t rv_csrrsi();
uint32_t rv_csrrci(uint8_t rd, uint8_t zimm, uint32_t csr_num);
uint32_t rv_csrrci();

uint32_t rv_mret();
uint32_t rv_wfi();

#endif
