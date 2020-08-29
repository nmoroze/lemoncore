#include <stdint.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "verilated.h"
#include "riscv.h"

#include "lemoncore.h"

// where vcd dumps are stored
#define WAVE_OUT_DIR "sim/"

class LemoncoreTest : public ::testing::Test {
protected:
  void SetUp() override {
    auto test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();

    std::ostringstream stream;
    stream << WAVE_OUT_DIR << "LemoncoreTest-" << test_name << ".vcd";
    std::string vcd_path = stream.str();

    cpu = new Lemoncore(false, vcd_path);
  }
  void TearDown() override {
    delete cpu;
  }
  Lemoncore* cpu;
};

TEST_F(LemoncoreTest, Basic) {
  cpu->write_imem(0, rv_addi(1, 0, 5)); // addi x1, x0, 5
  ASSERT_TRUE(cpu->run(6));
  EXPECT_EQ(cpu->get_reg(1), 5);
}

TEST_F(LemoncoreTest, IllegalInstruction) {
  cpu->write_imem(0, rv_addi(0, 0, 0)); // addi x0, x0, 0
  cpu->write_imem(4, 0);                // illegal

  ASSERT_TRUE(cpu->run(8));
  EXPECT_EQ(cpu->get_pc(), 0);
  EXPECT_EQ(cpu->get_mcause(), 2);
}

TEST_F(LemoncoreTest, MisalignedAddr) {
  cpu->write_imem(0, rv_jalr(0, 0, 2)); // jalr x0, 2(x0)
  ASSERT_TRUE(cpu->run(7));
  EXPECT_EQ(cpu->get_pc(), 0);
  EXPECT_EQ(cpu->get_mcause(), 0);
  EXPECT_EQ(cpu->get_mtval(), 2);
}

TEST_F(LemoncoreTest, MisalignedLoad) {
  cpu->write_imem(0, rv_lw(0, 0, 1)); // lw x0, 1(x0)
  ASSERT_TRUE(cpu->run(6));
  EXPECT_EQ(cpu->get_pc(), 0);
  EXPECT_EQ(cpu->get_mcause(), 4);
  EXPECT_EQ(cpu->get_mtval(), 1);
}

TEST_F(LemoncoreTest, MisalignedStore) {
  cpu->write_imem(0, rv_sw(0, 0, 1)); // sw x0, 1(x0)
  ASSERT_TRUE(cpu->run(6));
  EXPECT_EQ(cpu->get_pc(), 0);
  EXPECT_EQ(cpu->get_mcause(), 6);
  EXPECT_EQ(cpu->get_mtval(), 1);
}

TEST_F(LemoncoreTest, CSR) {
  cpu->write_imem(0, rv_csrrwi(1, 5, RV_CSR_MSCRATCH));
  cpu->write_imem(4, rv_csrrs(1, 0, RV_CSR_MSCRATCH));
  ASSERT_TRUE(cpu->run(10));
  EXPECT_EQ(cpu->get_reg(1), 5);
  EXPECT_EQ(cpu->get_mscratch(), 5);
}

TEST_F(LemoncoreTest, TimerIRQ) {
  cpu->set_mstatus(1 << 3);
  cpu->set_mie(1 << 7);
  cpu->write_imem(0, rv_addi(1, 0, 5));
  ASSERT_TRUE(cpu->step());
  ASSERT_TRUE(cpu->step());
  cpu->set_irq_timer(1);
  ASSERT_TRUE(cpu->step());
  EXPECT_EQ(cpu->get_mcause(), 1 << 31 | 7);
  EXPECT_EQ(cpu->get_mip(), 1 << 7);
  EXPECT_EQ(cpu->get_mstatus() & (1 << 3), 0);
  EXPECT_NE(cpu->get_reg(1), 5); // didn't finish the add
  EXPECT_EQ(cpu->get_pc(), 0);
}

TEST_F(LemoncoreTest, SoftwareIRQ) {
  cpu->set_mstatus(1 << 3);
  cpu->set_mie(1 << 3);
  cpu->write_imem(0, rv_addi(1, 0, 5));
  ASSERT_TRUE(cpu->step());
  ASSERT_TRUE(cpu->step());
  cpu->set_irq_software(1);
  ASSERT_TRUE(cpu->step());
  EXPECT_EQ(cpu->get_mcause(), 1 << 31 | 3);
  EXPECT_EQ(cpu->get_mip(), 1 << 3);
  EXPECT_EQ(cpu->get_mstatus() & (1 << 3), 0);
  EXPECT_NE(cpu->get_reg(1), 5); // didn't finish the add
  EXPECT_EQ(cpu->get_pc(), 0);
}

TEST_F(LemoncoreTest, ExternalIRQ) {
  cpu->set_mstatus(1 << 3);
  cpu->set_mie(1 << 11);
  cpu->write_imem(0, rv_addi(1, 0, 5));
  ASSERT_TRUE(cpu->step());
  ASSERT_TRUE(cpu->step());
  cpu->set_irq_external(1);
  ASSERT_TRUE(cpu->step());
  EXPECT_EQ(cpu->get_mcause(), 1 << 31 | 11);
  EXPECT_EQ(cpu->get_mip(), 1 << 11);
  EXPECT_EQ(cpu->get_mstatus() & (1 << 3), 0);
  EXPECT_NE(cpu->get_reg(1), 5); // didn't finish the add
  EXPECT_EQ(cpu->get_pc(), 0);
}

TEST_F(LemoncoreTest, InstructionCounter) {
  cpu->write_imem(0, rv_addi(1, 1, 1));
  cpu->write_imem(4, rv_blt(1, 2, -4));
  cpu->write_imem(8, rv_csrrw(3, 0, RV_CSR_INSTRET));
  cpu->write_imem(12, rv_csrrw(4, 0, RV_CSR_INSTRETH));
  cpu->set_reg(2, 10);
  const int bound = 300;
  int cycle = 0;
  while (cycle < bound && cpu->get_pc() != 16) {
    ASSERT_TRUE(cpu->step());
    cycle++;
  }

  // Make sure we didn't just time out
  ASSERT_LT(cycle, bound);

  ASSERT_EQ(cpu->get_reg(3), 20);
  ASSERT_EQ(cpu->get_reg(4), 0);
}

TEST_F(LemoncoreTest, CycleCounter) {
  cpu->set_mstatus(1 << 3);
  cpu->set_mie(1 << 11);

  cpu->write_imem(0, rv_csrrw(1, 0, RV_CSR_CYCLE));
  cpu->write_imem(4, rv_jal(0, 0));

  ASSERT_TRUE(cpu->run(24));
  cpu->set_irq_external(1); // force jump to IRQ vector (0)
  cpu->run(5); // run enough to execute csr read

  ASSERT_EQ(cpu->get_reg(1), 24 + 3);
}

TEST_F(LemoncoreTest, ExceptionHandler) {
  ASSERT_TRUE(cpu->load_firmware("sw/tests/test-exception-handler.bin"));

  const int bound = 3000;
  int cycle = 0;
  while (cycle < bound && cpu->get_reg(31) != 1) {
    ASSERT_TRUE(cpu->step());
    cycle++;
  }

  EXPECT_EQ(cpu->get_reg(1), 25);
  EXPECT_NE(cycle, bound);
}

TEST_F(LemoncoreTest, InsertionSort) {
  // Test data: random sequence of 25 numbers between 1 and 100
  const int num_numbers = 25;
  int numbers[num_numbers] = {24, 43, 18, 4, 91, 40, 100, 97, 41, 84, 13, 78,
                              99, 96, 19, 45, 11, 47, 22, 61, 66, 38, 29, 8, 25};
  int sorted_numbers[num_numbers] = {4, 8, 11, 13, 18, 19, 22, 24, 25, 29, 38,
                                     40, 41, 43, 45, 47, 61, 66, 78, 84, 91, 96,
                                     97, 99, 100};

  // Set up arguments
  for (int i = 0; i < num_numbers; i++) {
    cpu->write_ram(4 * i, numbers[i]);
  }
  cpu->set_reg(10, 0x1000); // array addr
  cpu->set_reg(11, num_numbers); // array len

  // Run
  ASSERT_TRUE(cpu->load_firmware("sw/tests/test-insertion-sort.bin"));

  const int bound = 6000;
  int cycles = 0;
  while (cycles < bound && cpu->get_reg(31) != 1) {
    ASSERT_TRUE(cpu->step());
    cycles++;
  }

  // Make sure we didn't just time out
  ASSERT_LT(cycles, bound);

  // Check sorted array
  for (int i = 0; i < num_numbers; i++) {
    uint32_t num = cpu->read_ram(4 * i);
    EXPECT_EQ(num, sorted_numbers[i]);
  }
}
