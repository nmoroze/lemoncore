#include <stdlib.h>
#include <gtest/gtest.h>
#include "Vregfile.h"
#include "verilated.h"

class RegfileTest : public ::testing::Test {
protected:
  void SetUp() override {
    tb = new Vregfile;
  }
  void TearDown() override {
    delete tb;
  }
  void Step() {
    // Run one clock cycle (posedge)
    tb->clk_i = 0;
    tb->eval();
    tb->clk_i = 1;
    tb->eval();
    tb->clk_i = 0;
  }
  Vregfile *tb;
};

TEST_F(RegfileTest, InitiallyZero) {
  tb->rs1_i = 5;
  tb->rs2_i = 8;
  tb->eval();

  EXPECT_EQ(tb->rd1_o, 0);
  EXPECT_EQ(tb->rd2_o, 0);
}

TEST_F(RegfileTest, Write) {
  tb->we_i = 1;
  tb->ws_i = 5;
  tb->wd_i = 42;
  Step();

  tb->rs1_i = 5;
  Step();
  tb->eval();

  EXPECT_EQ(tb->rd1_o, 42);
}

TEST_F(RegfileTest, WriteToZeroDropped) {
  tb->we_i = 1;
  tb->ws_i = 0;
  tb->wd_i = 42;
  Step();

  tb->rs1_i = 0;
  tb->eval();

  EXPECT_EQ(tb->rd1_o, 0);
}
