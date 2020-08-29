#include <stdlib.h>
#include <gtest/gtest.h>
#include "Vext.h"
#include "verilated.h"

#define EXT_SEL_B  0b000;
#define EXT_SEL_H  0b001;
#define EXT_SEL_W  0b010;
#define EXT_SEL_BU 0b100;
#define EXT_SEL_HU 0b101;

class ExtTest : public ::testing::Test {
protected:
  void SetUp() override {
    tb = new Vext;
  }
  void TearDown() override {
    delete tb;
  }
  Vext *tb;
};

TEST_F(ExtTest, MSBChopped) {
 	tb->in_i = 0x8001;
	tb->sel_i = EXT_SEL_BU;
	tb->eval();

	EXPECT_EQ(tb->out_o, 1);
}

TEST_F(ExtTest, WordStaysSame) {
	tb->in_i = 0x8001;
	tb->sel_i = EXT_SEL_W;
	tb->eval();

	EXPECT_EQ(tb->out_o, 0x8001);
}

TEST_F(ExtTest, SignPreserved) {
	tb->in_i = 0xFFFF;
	tb->sel_i = EXT_SEL_H;
	tb->eval();

	EXPECT_EQ(tb->out_o, -1);
}
