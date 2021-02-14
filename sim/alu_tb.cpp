#include <stdlib.h>
#include <gtest/gtest.h>
#include "Valu.h"
#include "verilated.h"

#define ALU_OP_ADD  0b000;
#define ALU_OP_SHL  0b001;
#define ALU_OP_CMP  0b010;
#define ALU_OP_CMPU 0b011;
#define ALU_OP_XOR  0b100;
#define ALU_OP_SHR  0b101;
#define ALU_OP_OR   0b110;
#define ALU_OP_AND  0b111;

class AluTest : public ::testing::Test {
protected:
  void SetUp() override {
    tb = new Valu;
  }
  void TearDown() override {
    delete tb;
  }
  Valu *tb;
};

TEST_F(AluTest, Addition) {
	tb->a_i = 5;
	tb->b_i = 7;
  tb->op_i = ALU_OP_ADD;
  tb->eval();

	EXPECT_EQ(tb->out_o, 12);
}

TEST_F(AluTest, SignedComparison) {
 	tb->a_i = -1;
	tb->b_i = 1;
  tb->op_i = ALU_OP_CMP;
  tb->eval();

	EXPECT_EQ(tb->out_o, 1);
}

TEST_F(AluTest, UnsignedComparision) {
  tb->a_i = -1;
  tb->b_i = 1;
  tb->op_i = ALU_OP_CMPU;
  tb->eval();

  EXPECT_EQ(tb->out_o, 0);
}

TEST_F(AluTest, ArithmeticVsLogicalShift) {
  tb->a_i = -1;
  tb->b_i = 31;
  tb->op_i = ALU_OP_SHR;
  tb->shift_type_i = 1; // arithmetic
  tb->eval();
  EXPECT_EQ(tb->out_o, -1);

  tb->shift_type_i = 0; // logical
  tb->eval();
  EXPECT_EQ(tb->out_o, 1);
}
