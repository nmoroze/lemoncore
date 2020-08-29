#include "riscv.h"

#include <stdint.h>

uint32_t inline MASK(uint32_t val, uint32_t width) {
  return (val & ((1 << width) - 1));
}

uint32_t inline MASK_RANGE(uint32_t val, uint32_t end, uint32_t start) {
  return (MASK(val >> start, end - start + 1));
}

uint32_t MASK_BIT(uint32_t val, uint32_t bit) {
  return (MASK_RANGE(val, bit, bit));
}

uint32_t type_u(uint8_t op, uint8_t rd, int32_t imm) {
  return MASK_RANGE(imm, 31, 12) << 12 | MASK(rd, 5) << 7 | op;
}

uint32_t type_i(uint8_t op, uint8_t rd, uint8_t func, uint8_t rs1, int32_t imm) {
  return op | ((rd & 0x1f) << 7) | ((func & 7) << 12) | ((rs1 & 0x1f) << 15)
    | ((imm & 0xfff) << 20);
}

uint32_t type_s(uint8_t func, uint8_t rs1, uint8_t rs2, int32_t imm) {
  return MASK_RANGE(imm, 11, 5) << 25 | MASK(rs2, 5) << 20 | MASK(rs1, 5) << 15 |
    MASK(func, 3) << 12 | MASK_RANGE(imm, 4, 0) << 7 | 0b0100011;
}

uint32_t type_b(uint8_t func, uint8_t rs1, uint8_t rs2, int32_t imm) {
  return MASK_BIT(imm, 12) << 31 | MASK_RANGE(imm, 10, 5) << 25 |
    MASK(rs2, 5) << 20 | MASK(rs1, 5) << 15 | MASK(func, 3) << 12 |
    MASK_RANGE(imm, 4, 1) << 8 | MASK_BIT(imm, 11) << 7 | 0b1100011;
}

uint32_t type_r(uint8_t rd, uint8_t func, uint8_t rs1, uint8_t rs2, uint8_t op_bit) {
  return MASK(op_bit, 1) << 30 | MASK(rs2, 5) << 20 | MASK(rs1, 5) << 15 |
    MASK(func, 3) << 12 | MASK(rd, 5) << 7 | 0b0110011;
}

uint32_t rv_lui(uint8_t rd, int32_t imm) {
  return type_u(0b0110111, rd, imm);
}

uint32_t rv_lui() {
  return rv_lui(0, 0);
}

uint32_t rv_auipc(uint8_t rd, int32_t imm) {
  return type_u(0b0010111, rd, imm);
}

uint32_t rv_auipc() {
  return rv_auipc(0, 0);
}

uint32_t rv_jal(uint32_t rd, int32_t imm) {
  uint32_t swizzled_imm = MASK_RANGE(imm, 19, 12) | MASK_BIT(imm, 11) << 8 |
    MASK_RANGE(imm, 10, 1) << 9 | MASK_BIT(imm, 20) << 19;
  return swizzled_imm << 12 | MASK(rd, 5) << 7 | 0b1101111;
}

uint32_t rv_jal() {
  return rv_jal(0, 0);
}

uint32_t rv_jalr(uint32_t rd, uint32_t rs1, int32_t imm) {
  return type_i(0b1100111, rd, 0, rs1, imm);
}

uint32_t rv_jalr() {
  return rv_jalr(0, 0, 0);
}

uint32_t rv_beq(uint8_t rs1, uint8_t rs2, int32_t imm) {
  return type_b(0b000, rs1, rs2, imm);
}

uint32_t rv_beq() {
  return rv_beq(0, 0, 0);
}

uint32_t rv_bne(uint8_t rs1, uint8_t rs2, int32_t imm) {
  return type_b(0b001, rs1, rs2, imm);
}

uint32_t rv_bne() {
  return rv_bne(0, 0, 0);
}

uint32_t rv_blt(uint8_t rs1, uint8_t rs2, int32_t imm) {
  return type_b(0b100, rs1, rs2, imm);
}

uint32_t rv_blt() {
  return rv_blt(0, 0, 0);
}

uint32_t rv_bge(uint8_t rs1, uint8_t rs2, int32_t imm) {
  return type_b(0b101, rs1, rs2, imm);
}

uint32_t rv_bge() {
  return rv_bge(0, 0, 0);
}

uint32_t rv_bltu(uint8_t rs1, uint8_t rs2, int32_t imm) {
  return type_b(0b110, rs1, rs2, imm);
}

uint32_t rv_bltu() {
  return rv_bltu(0, 0, 0);
}

uint32_t rv_bgeu(uint8_t rs1, uint8_t rs2, int32_t imm) {
  return type_b(0b111, rs1, rs2, imm);
}

uint32_t rv_bgeu() {
  return rv_bgeu(0, 0, 0);
}

uint32_t rv_lb(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0000011, rd, 0b000, rs1, imm);
}

uint32_t rv_lb() {
  return rv_lb(0, 0, 0);
}

uint32_t rv_lh(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0000011, rd, 0b001, rs1, imm);
}

uint32_t rv_lh() {
  return rv_lh(0, 0, 0);
}

uint32_t rv_lw(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0000011, rd, 0b010, rs1, imm);
}

uint32_t rv_lw() {
  return rv_lw(0, 0, 0);
}

uint32_t rv_lbu(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0000011, rd, 0b100, rs1, imm);
}

uint32_t rv_lbu() {
  return rv_lbu(0, 0, 0);
}

uint32_t rv_lhu(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0000011, rd, 0b101, rs1, imm);
}

uint32_t rv_lhu() {
  return rv_lhu(0, 0, 0);
}

uint32_t rv_sb(uint8_t rs2, uint8_t rs1, int32_t imm) {
  return type_s(0b000, rs1, rs2, imm);
}

uint32_t rv_sb() {
  return rv_sb(0, 0, 0);
}

uint32_t rv_sh(uint8_t rs2, uint8_t rs1, int32_t imm) {
  return type_s(0b001, rs1, rs2, imm);
}

uint32_t rv_sh() {
  return rv_sh(0, 0, 0);
}

uint32_t rv_sw(uint8_t rs2, uint8_t rs1, int32_t imm) {
  return type_s(0b010, rs1, rs2, imm);
}

uint32_t rv_sw() {
  return rv_sw(0, 0, 0);
}

uint32_t rv_addi(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b000, rs1, imm);
}

uint32_t rv_addi() {
  return rv_addi(0, 0, 0);
}

uint32_t rv_slti(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b010, rs1, imm);
}

uint32_t rv_slti() {
  return rv_slti(0, 0, 0);
}

uint32_t rv_sltiu(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b011, rs1, imm);
}

uint32_t rv_sltiu() {
  return rv_sltiu(0, 0, 0);
}

uint32_t rv_xori(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b100, rs1, imm);
}

uint32_t rv_xori() {
  return rv_xori(0, 0, 0);
}

uint32_t rv_ori(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b110, rs1, imm);
}

uint32_t rv_ori() {
  return rv_ori(0, 0, 0);
}

uint32_t rv_andi(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b111, rs1, imm);
}

uint32_t rv_andi() {
  return rv_andi(0, 0, 0);
}

uint32_t rv_slli(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b001, rs1, MASK(imm, 5));
}

uint32_t rv_slli() {
  return rv_slli(0, 0, 0);
}

uint32_t rv_srli(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b101, rs1, MASK(imm, 5));
}

uint32_t rv_srli() {
  return rv_srli(0, 0, 0);
}

uint32_t rv_srai(uint8_t rd, uint8_t rs1, int32_t imm) {
  return type_i(0b0010011, rd, 0b101, rs1, (1 << 30) | MASK(imm, 5));
}

uint32_t rv_srai() {
  return rv_srai(0, 0, 0);
}

uint32_t rv_add(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b000, rs1, rs2, 0);
}

uint32_t rv_add() {
  return rv_add(0, 0, 0);
}

uint32_t rv_sub(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b000, rs1, rs2, 1);
}

uint32_t rv_sub() {
  return rv_sub(0, 0, 0);
}

uint32_t rv_sll(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b001, rs1, rs2, 0);
}

uint32_t rv_sll() {
  return rv_sll(0, 0, 0);
}

uint32_t rv_slt(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b010, rs1, rs2, 0);
}

uint32_t rv_slt() {
  return rv_slt(0, 0, 0);
}

uint32_t rv_sltu(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b011, rs1, rs2, 0);
}

uint32_t rv_sltu() {
  return rv_sltu(0, 0, 0);
}

uint32_t rv_xor(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b100, rs1, rs2, 0);
}

uint32_t rv_xor() {
  return rv_xor(0, 0, 0);
}

uint32_t rv_srl(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b101, rs1, rs2, 0);
}

uint32_t rv_srl() {
  return rv_srl(0, 0, 0);
}

uint32_t rv_sra(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b101, rs1, rs2, 1);
}

uint32_t rv_sra() {
  return rv_sra(0, 0, 0);
}

uint32_t rv_or(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b110, rs1, rs2, 0);
}

uint32_t rv_or() {
  return rv_or(0, 0, 0);
}

uint32_t rv_and(uint8_t rd, uint8_t rs1, uint8_t rs2) {
  return type_r(rd, 0b111, rs1, rs2, 0);
}

uint32_t rv_and() {
  return rv_and(0, 0, 0);
}

uint32_t csr(uint8_t rd, uint8_t func, uint8_t rs1_zimm, uint32_t csr_num) {
  return MASK(csr_num, 12) << 20 | MASK(rs1_zimm, 5) << 15 | MASK(func, 3) << 12
    | MASK(rd, 5) << 7 | 0b1110011;
}

uint32_t rv_csrrw(uint8_t rd, uint8_t rs1, uint32_t csr_num) {
  return csr(rd, 0b001, rs1, csr_num);
}

uint32_t rv_csrrw() {
  return rv_csrrw(0, 0, RV_CSR_MISA);
}

uint32_t rv_csrrs(uint8_t rd, uint8_t rs1, uint32_t csr_num) {
  return csr(rd, 0b010, rs1, csr_num);
}

uint32_t rv_csrrs() {
  return rv_csrrs(0, 0, RV_CSR_MISA);
}

uint32_t rv_csrrc(uint8_t rd, uint8_t rs1, uint32_t csr_num) {
  return csr(rd, 0b011, rs1, csr_num);
}

uint32_t rv_csrrc() {
  return rv_csrrc(0, 0, RV_CSR_MISA);
}

uint32_t rv_csrrwi(uint8_t rd, uint8_t zimm, uint32_t csr_num) {
  return csr(rd, 0b101, zimm, csr_num);
}

uint32_t rv_csrrwi() {
  return rv_csrrwi(0, 0, RV_CSR_MISA);
}

uint32_t rv_csrrsi(uint8_t rd, uint8_t zimm, uint32_t csr_num) {
  return csr(rd, 0b110, zimm, csr_num);
}

uint32_t rv_csrrsi() {
  return rv_csrrsi(0, 0, RV_CSR_MISA);
}

uint32_t rv_csrrci(uint8_t rd, uint8_t zimm, uint32_t csr_num) {
  return csr(rd, 0b111, zimm, csr_num);
}

uint32_t rv_csrrci() {
  return rv_csrrci(0, 0, RV_CSR_MISA);
}

uint32_t rv_fence(uint8_t pred, uint8_t succ) {
  return MASK(pred, 4) << 24 | MASK(succ, 4) << 20 | 0b0001111;
}

uint32_t rv_fence() {
  return rv_fence(0, 0);
}

uint32_t rv_fence_i() {
  return 1 << 12 | 0b0001111;
}

uint32_t rv_ecall() {
  return 0b1110011;
}

uint32_t rv_ebreak() {
  return 1 << 20 | 0b1110011;
}

uint32_t rv_mret() {
  return 0b0011000 << 25 | 0b00010 << 20 | 0b1110011;
}

uint32_t rv_wfi() {
  return 0b0001000 << 25 | 0b00101 << 20 | 0b1110011;
}
