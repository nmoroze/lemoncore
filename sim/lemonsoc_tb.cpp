#include <array>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "riscv.h"
#include "verilated.h"

#include "lemonsoc.h"

// where vcd dumps are stored
#define WAVE_OUT_DIR "sim/"

class LemonsocTest : public ::testing::Test {
protected:
  void SetUp() override {
    auto test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

    std::ostringstream stream;
    stream << WAVE_OUT_DIR << "LemonsocTest-" << test_name << ".vcd";
    std::string vcd_path = stream.str();

    soc = new Lemonsoc(false, true, vcd_path);
  }
  void TearDown() override {
    delete soc;
  }
  Lemonsoc* soc;
};

TEST_F(LemonsocTest, Main) {
  // Smoke test: make sure LED flickers within a roughly correct # of cycles
  // we don't specify an exact timing because it varies with small changes to
  // the software library.
  EXPECT_TRUE(soc->load_firmware("sw/hello.sim.mem"));

  auto leds = soc->get_leds();
  for (int i = 0; i < 5; i++)
    EXPECT_EQ(leds[i], 0);
  int cycles = 0;
  while (soc->get_led(1) == 0 && cycles < 100000) {
    soc->step();
    cycles++;
  }
  EXPECT_LT(cycles, 100000);
  EXPECT_GT(cycles, 90000);

  cycles = 0;
  soc->set_btns(0, 0, 1);
  soc->run(200);
  soc->set_btns(0, 0, 1);
  soc->run(200);

  while (soc->get_led(1) == 1 && cycles < 100000) {
    soc->step();
    cycles++;
  }
  EXPECT_LT(cycles, 80000);
  EXPECT_GT(cycles, 70000);
}

TEST_F(LemonsocTest, PartialStore) {
  soc->set_reg(1, 0x42);
  soc->set_reg(2, 0x2000); // data mem base, TODO: replace w constant
  soc->write_imem(0, rv_sb(1, 2, 1));
  soc->write_imem(4, rv_lw(3, 2, 0));
  soc->run_till_pc(8);
  EXPECT_EQ(soc->get_reg(3), 0x4200);

  // partial stores shouldn't overwrite other stuff
  soc->set_reg(1, 0x42);
  soc->set_reg(2, 0x2000); // data mem base, TODO: replace w constant
  soc->set_reg(3, 0x12345678);
  soc->write_imem(8, rv_sw(3, 2, 0));
  soc->write_imem(12, rv_sb(1, 2, 1));
  soc->write_imem(16, rv_lw(3, 2, 0));
  soc->run_till_pc(20);
  EXPECT_EQ(soc->get_reg(3), 0x12344278);

  soc->set_reg(1, 0x4321);
  soc->set_reg(2, 0x2000);
  soc->set_reg(3, 0x12345678);
  soc->write_imem(20, rv_sw(3, 2, 0));
  soc->write_imem(24, rv_sh(1, 2, 0));
  soc->write_imem(28, rv_lw(3, 2, 0));
  soc->run_till_pc(32);
  EXPECT_EQ(soc->get_reg(3), 0x12344321);

  soc->set_reg(1, 0x4321);
  soc->set_reg(2, 0x2000);
  soc->set_reg(3, 0x12345678);
  soc->write_imem(32, rv_sw(3, 2, 0));
  soc->write_imem(36, rv_sh(1, 2, 2));
  soc->write_imem(40, rv_lw(3, 2, 0));
  soc->run_till_pc(44);
  EXPECT_EQ(soc->get_reg(3), 0x43215678);

  // Partial stores ignore upper portion of rs1
  soc->set_reg(1, 0x43210000);
  soc->set_reg(2, 0x2000);
  soc->set_reg(3, 0x12345678);
  soc->write_imem(44, rv_sw(3, 2, 0));
  soc->write_imem(48, rv_sh(1, 2, 0));
  soc->write_imem(52, rv_lw(3, 2, 0));
  soc->run_till_pc(56);
  EXPECT_EQ(soc->get_reg(3), 0x12340000);

  // Partial stores are always unsigned
  soc->set_reg(1, -1);
  soc->set_reg(2, 0x2000);
  soc->write_imem(56, rv_sw(0, 2, 0));
  soc->write_imem(60, rv_sb(1, 2, 0));
  soc->write_imem(64, rv_sh(1, 2, 4));
  soc->write_imem(68, rv_lw(3, 2, 0));
  soc->write_imem(72, rv_lw(4, 2, 4));
  soc->run_till_pc(76);
  EXPECT_EQ(soc->get_reg(3), 0xFF);
  EXPECT_EQ(soc->get_reg(4), 0xFFFF);
}

TEST_F(LemonsocTest, PartialLoads) {
  soc->set_reg(1, 0x12345678);
  soc->set_reg(2, 0x2000); // data mem base, TODO: replace w constant
  soc->write_imem(0, rv_sw(1, 2, 0)); // sw x1, 0(x2)
  soc->write_imem(4, rv_lbu(3, 2, 0));
  soc->write_imem(8, rv_lbu(4, 2, 1));
  soc->write_imem(12, rv_lbu(5, 2, 2));
  soc->write_imem(16, rv_lbu(6, 2, 3));
  soc->write_imem(20, rv_lhu(7, 2, 0));
  soc->write_imem(24, rv_lhu(8, 2, 2));
  soc->run_till_pc(28);

  EXPECT_EQ(soc->get_reg(3), 0x78);
  EXPECT_EQ(soc->get_reg(4), 0x56);
  EXPECT_EQ(soc->get_reg(5), 0x34);
  EXPECT_EQ(soc->get_reg(6), 0x12);
  EXPECT_EQ(soc->get_reg(7), 0x5678);
  EXPECT_EQ(soc->get_reg(8), 0x1234);

  soc->set_reg(1, 1);
  soc->write_imem(28, rv_sw(1, 2, 0));
  soc->write_imem(32, rv_lb(3, 2, 0));
  soc->write_imem(36, rv_lh(4, 2, 0));

  soc->run_till_pc(40);

  EXPECT_EQ(soc->get_reg(3), 1);
  EXPECT_EQ(soc->get_reg(4), 1);

  soc->set_reg(1, -1);
  soc->write_imem(40, rv_sw(1, 2, 0));
  soc->write_imem(44, rv_lb(3, 2, 0));
  soc->write_imem(48, rv_lh(4, 2, 0));

  soc->run_till_pc(52);

  EXPECT_EQ(soc->get_reg(3), -1);
  EXPECT_EQ(soc->get_reg(4), -1);

}
