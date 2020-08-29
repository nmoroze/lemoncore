#ifndef LEMONCORE_H
#define LEMONCORE_H

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "Vlemoncore.h"

#define ROM_SIZE 4096  // bytes
#define RAM_SIZE 8208  // bytes, includes RAM and peripherals

class Lemoncore {
 public:
  explicit Lemoncore(bool verbose);
  Lemoncore(bool verbose, std::string vcd_path);
  ~Lemoncore();
  bool load_firmware(std::string path);
  void reset();
  bool step();
  bool run(int cycles);
  bool run_till_pc(uint32_t pc);
  void set_reg(uint8_t reg, uint32_t data);
  uint32_t get_reg(uint8_t reg);
  uint32_t get_pc();
  uint32_t get_mcause();
  uint32_t get_mstatus();
  void set_mstatus(uint32_t mstatus);
  uint32_t get_mie();
  void set_mie(uint32_t mie);
  uint32_t get_mip();
  uint32_t get_mtval();
  uint32_t get_mscratch();
  void write_imem(uint32_t addr, uint32_t data);
  void write_ram(uint32_t addr, uint32_t data);
  uint32_t read_ram(uint32_t addr);
  void set_irq_timer(int val);
  void set_irq_software(int val);
  void set_irq_external(int val);
 private:
  void init(bool verbose, std::string vcd_path);
  void dump_regs();
  void log(const char* fmt...);

  uint32_t mem[(ROM_SIZE + RAM_SIZE) / 4];
  bool verbose;
  int cycle;
  Vlemoncore *tb;
  VerilatedVcdC* tfp;
};

#endif
