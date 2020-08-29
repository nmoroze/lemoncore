#ifndef LEMONSOC_H
#define LEMONSOC_H

#include <stdlib.h>
#include <iostream>
#include "Vlemonsoc.h"

class Lemonsoc {
 public:
  explicit Lemonsoc(bool verbose);
  Lemonsoc(bool verbose, std::string vcd_path);
  ~Lemonsoc();
  bool load_firmware(std::string path);
  void reset();
  bool step();
  bool run(int cycles);
  void set_btns(bool btn1, bool btn2, bool btn3);
  int get_led(int led);
  bool is_done();
  std::array<int, 5> get_leds();
  void set_reg(uint8_t reg, uint32_t data);
  void write_imem(uint32_t addr, uint32_t data);
  bool run_till_pc(uint32_t pc);
  uint32_t get_pc();
  uint32_t get_reg(uint8_t reg);
 private:
  void init(bool verbose, std::string vcd_path);
  void log(const char* fmt...);

  bool verbose;
  int cycle;
  Vlemonsoc *tb;
  VerilatedVcdC* tfp;
};

#endif
