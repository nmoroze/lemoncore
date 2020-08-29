#include <stdint.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "Vdecoder.h"
#include "verilated.h"
#include "riscv.h"

#include "decoder_tb.h"

const static int NUM_INSTRUCTIONS = 37;

struct control_signals_t {
  std::string instr;
  uint32_t (*gen_instr)();
  int alu_op;
  int a_src;
  int b_src;
  int b_neg;
  int mem_w;
  int reg_w;
  int next_pc;
  int wb_src;
  int shift_type;
};

struct control_signals_t CONTROL_SIGNAL_TABLE[NUM_INSTRUCTIONS] = {
  // Instr           ALUop        Asrc       Bsrc       Bneg  MemW  RegW  NextPc       WBsrc       ShiftType
 {"LUI",   rv_lui,   ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"AUIPC", rv_auipc, ALU_OP_ADD,  A_SRC_PC,  B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"JAL",   rv_jal,   ALU_OP_AND,  A_SRC_RS1, -1,        -1,   0,    1,    NEXT_PC_BR0, WB_SRC_PC,  -1},
 {"JALR",  rv_jalr,  ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_ALU, WB_SRC_PC,  -1},
 {"BEQ",   rv_beq,   ALU_OP_XOR,  A_SRC_RS1, B_SRC_RS2, 0,    0,    0,    NEXT_PC_BR0, -1,         -1},
 {"BNE",   rv_bne,   ALU_OP_XOR,  A_SRC_RS1, B_SRC_RS2, 0,    0,    0,    NEXT_PC_BR1, -1,         -1},
 {"BLT",   rv_blt,   ALU_OP_CMP,  A_SRC_RS1, B_SRC_RS2, 0,    0,    0,    NEXT_PC_BR1, -1,         -1},
 {"BGE",   rv_bge,   ALU_OP_CMP,  A_SRC_RS1, B_SRC_RS2, 0,    0,    0,    NEXT_PC_BR0, -1,         -1},
 {"BLTU",  rv_bltu,  ALU_OP_CMPU, A_SRC_RS1, B_SRC_RS2, 0,    0,    0,    NEXT_PC_BR1, -1,         -1},
 {"BGEU",  rv_bgeu,  ALU_OP_CMPU, A_SRC_RS1, B_SRC_RS2, 0,    0,    0,    NEXT_PC_BR0, -1,         -1},
 {"LB",    rv_lb,    ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_MEM, -1},
 {"LH",    rv_lh,    ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_MEM, -1},
 {"LW",    rv_lw,    ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_MEM, -1},
 {"LBU",   rv_lbu,   ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_MEM, -1},
 {"LHU",   rv_lhu,   ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_MEM, -1},
 {"SB",    rv_sb,    ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    1,    0,    NEXT_PC_INC, -1,         -1},
 {"SH",    rv_sh,    ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    1,    0,    NEXT_PC_INC, -1,         -1},
 {"SW",    rv_sw,    ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    1,    0,    NEXT_PC_INC, -1,         -1},
 {"ADDI",  rv_addi,  ALU_OP_ADD,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SLTI",  rv_slti,  ALU_OP_CMP,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SLTIU", rv_sltiu, ALU_OP_CMPU, A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"XORI",  rv_xori,  ALU_OP_XOR,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"ORI",   rv_ori,   ALU_OP_OR,   A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"ANDI",  rv_andi,  ALU_OP_AND,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SLLI",  rv_slli,  ALU_OP_SHL,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SRLI",  rv_srli,  ALU_OP_SHR,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, 1},
 {"SRAI",  rv_srai,  ALU_OP_SHR,  A_SRC_RS1, B_SRC_IMM, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, 0},
 {"ADD",   rv_add,   ALU_OP_ADD,  A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SUB",   rv_sub,   ALU_OP_ADD,  A_SRC_RS1, B_SRC_RS2, 1,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SLL",   rv_sll,   ALU_OP_SHL,  A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SLT",   rv_slt,   ALU_OP_CMP,  A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SLTU",  rv_sltu,  ALU_OP_CMPU, A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"XOR",   rv_xor,   ALU_OP_XOR,  A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"SRL",   rv_srl,   ALU_OP_SHR,  A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, 1},
 {"SRA",   rv_sra,   ALU_OP_SHR,  A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, 0},
 {"OR",    rv_or,    ALU_OP_OR,   A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
 {"AND",   rv_and,   ALU_OP_AND,  A_SRC_RS1, B_SRC_RS2, 0,    0,    1,    NEXT_PC_INC, WB_SRC_ALU, -1},
};

class DecoderTest : public ::testing::Test {
protected:
  void SetUp() override {
    tb = new Vdecoder;
  }
  void TearDown() override {
    delete tb;
  }
  Vdecoder *tb;
};

TEST_F(DecoderTest, ControlSignals) {
  for (int i = 0; i < NUM_INSTRUCTIONS; i++) {
    auto ctrlsigs = CONTROL_SIGNAL_TABLE[i];
    uint32_t instr = ctrlsigs.gen_instr();
    auto name = ctrlsigs.instr;

    tb->instr_i = instr;
    tb->eval();

    EXPECT_EQ(tb->illegal_instr_o, 0);

    if (ctrlsigs.alu_op != -1)
      EXPECT_EQ(tb->alu_op_o, ctrlsigs.alu_op) << "ALUOp incorrect for instruction " << name;
    if (ctrlsigs.a_src != -1)
      EXPECT_EQ(tb->a_src_o, ctrlsigs.a_src) << "Asrc incorrect for instruction " << name;
    if (ctrlsigs.b_src != -1)
      EXPECT_EQ(tb->b_src_o, ctrlsigs.b_src) << "Bsrc incorrect for instruction " << name;
    if (ctrlsigs.b_neg != -1)
      EXPECT_EQ(tb->negate_b_o, ctrlsigs.b_neg) << "NegateB incorrect for instruction " << name;
    if (ctrlsigs.mem_w != -1)
      EXPECT_EQ(tb->mem_w_o, ctrlsigs.mem_w) << "MemW incorrect for instruction " << name;
    if (ctrlsigs.reg_w != -1)
      EXPECT_EQ(tb->reg_w_o, ctrlsigs.reg_w) << "RegW incorrect for instruction " << name;
    if (ctrlsigs.next_pc != -1)
      EXPECT_EQ(tb->next_pc_o, ctrlsigs.next_pc) << "NextPC incorrect for instruction " << name;
    if (ctrlsigs.wb_src != -1)
      EXPECT_EQ(tb->wb_src_o, ctrlsigs.wb_src) << "WBsrc incorrect for instruction " << name;
  }
}

TEST_F(DecoderTest, IllegalInstruction) {
  tb->instr_i = 0x0;
  tb->eval();
  EXPECT_EQ(tb->illegal_instr_o, 1);

  tb->instr_i = rv_and() | (1 << 30); // AND w/ a twist
  tb->eval();
  EXPECT_EQ(tb->illegal_instr_o, 1);
}

TEST_F(DecoderTest, ExtractRs1) {
  // Regression test for silly bug where I mixed up jal and jalr when refactoring
  // decoder. Would be nice to include rs1 gating in main control signals test,
  // but not sure of super clean way to do it.
  tb->instr_i = rv_jalr(0, 1, 0); // jalr w/ rs1 = 1
  tb->eval();
  EXPECT_EQ(tb->rs1_o, 1);

  tb->instr_i = rv_jal() | (1 << 15);
  tb->eval();
  EXPECT_EQ(tb->rs1_o, 0);
}

TEST_F(DecoderTest, System) {
  // System instructions don't really take advantage of the same control signals,
  // so cleanest to test them individually.
  tb->instr_i = rv_fence();
  tb->eval();
  EXPECT_EQ(tb->nop_o, 1);
  EXPECT_EQ(tb->ebreak_o, 0);
  EXPECT_EQ(tb->ecall_o, 0);
  EXPECT_EQ(tb->mret_o, 0);

  tb->instr_i = rv_fence_i();
  tb->eval();
  EXPECT_EQ(tb->nop_o, 1);
  EXPECT_EQ(tb->ebreak_o, 0);
  EXPECT_EQ(tb->ecall_o, 0);
  EXPECT_EQ(tb->mret_o, 0);

  tb->instr_i = rv_wfi();
  tb->eval();
  EXPECT_EQ(tb->nop_o, 1);
  EXPECT_EQ(tb->ebreak_o, 0);
  EXPECT_EQ(tb->ecall_o, 0);
  EXPECT_EQ(tb->mret_o, 0);

  tb->instr_i = rv_ecall();
  tb->eval();
  EXPECT_EQ(tb->ecall_o, 1);
  EXPECT_EQ(tb->ebreak_o, 0);
  EXPECT_EQ(tb->mret_o, 0);
  EXPECT_EQ(tb->nop_o, 0);

  tb->instr_i = rv_ebreak();
  tb->eval();
  EXPECT_EQ(tb->ebreak_o, 1);
  EXPECT_EQ(tb->ecall_o, 0);
  EXPECT_EQ(tb->mret_o, 0);
  EXPECT_EQ(tb->nop_o, 0);

  tb->instr_i = rv_mret();
  tb->eval();
  EXPECT_EQ(tb->mret_o, 1);
  EXPECT_EQ(tb->ecall_o, 0);
  EXPECT_EQ(tb->ebreak_o, 0);
  EXPECT_EQ(tb->nop_o, 0);
}
