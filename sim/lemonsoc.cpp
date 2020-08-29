#include <array>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include "Vlemonsoc.h"
#include "Vlemonsoc_lemonsoc.h"
#include "Vlemonsoc_lemoncore.h"
#include "Vlemonsoc_regfile.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include <svdpi.h>
#include "Vlemonsoc__Dpi.h"

#include "lemonsoc.h"

#define DEFAULT_VCD_PATH "lemonsoc.vcd"

Lemonsoc::Lemonsoc(bool verbose) {
  init(verbose, DEFAULT_VCD_PATH);
}

Lemonsoc::Lemonsoc(bool verbose, std::string vcd_path) {
  init(verbose, vcd_path);
}

void Lemonsoc::init(bool verbose, std::string vcd_path) {
  this->verbose = verbose;
  tb = new Vlemonsoc;
  cycle = 0;

  //Verilated::scopesDump();
  svSetScope(svGetScopeFromName("TOP.lemonsoc.ram"));

  // Start tracing
  tfp = new VerilatedVcdC;
  Verilated::traceEverOn(true);
  tb->trace(tfp, 99);
  tfp->open(vcd_path.c_str());

  // Reset SoC
  reset();
}

Lemonsoc::~Lemonsoc() {
  // Stop tracing
  tfp->close();

  delete tb;
  delete tfp;
}

bool Lemonsoc::load_firmware(std::string path) {
  verilator_load_mem(path.c_str());
  return true;
}

void Lemonsoc::reset() {
  // Reset SoC
  tb->BTN_N = 0;
  tb->CLK = 0;
  tb->eval();
  tfp->dump(cycle);
  tb->CLK = 1;
  tb->eval();
  tb->BTN_N = 1;
  tfp->dump(cycle + 1);

  cycle++;
}

bool Lemonsoc::step() {
  tb->CLK = 0;
  tb->eval();
  tfp->dump(2 * cycle);
  tb->CLK = 1;
  tb->eval();
  tfp->dump(2 * cycle + 1);

  cycle++;

  if (tb->LEDR_N == 0) {
    return false;
  }

  return true;
}

bool Lemonsoc::run(int cycles) {
  for (int c = 0; c < cycles && !Verilated::gotFinish() && !is_done(); c++) {
    if (!step())
      return false;
  }

  return true;
}

void Lemonsoc::log(const char* fmt...) {
  // https://stackoverflow.com/q/41400
  if (verbose) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
}

void Lemonsoc::set_btns(bool btn1, bool btn2, bool btn3) {
  tb->BTN1 = btn1;
  tb->BTN2 = btn2;
  tb->BTN3 = btn3;
}

std::array<int, 5> Lemonsoc::get_leds() {
  return {tb->LED1, tb->LED2, tb->LED3, tb->LED4, tb->LED5};
}

int Lemonsoc::get_led(int led) {
  switch (led) {
  case 1:
    return tb->LED1;
  case 2:
    return tb->LED2;
  case 3:
    return tb->LED3;
  case 4:
    return tb->LED4;
  case 5:
    return tb->LED5;
  }

  assert(false); // only 4 LEDs
  return -1;
}

bool Lemonsoc::is_done() {
  return tb->LEDG_N == 0;
}

void Lemonsoc::set_reg(uint8_t reg, uint32_t data) {
  tb->lemonsoc->lemon->regfile->regs[reg] = data;
}

void Lemonsoc::write_imem(uint32_t addr, uint32_t data) {
  assert(addr % 4 == 0);
  //assert(addr < ROM_SIZE);
  verilator_set_mem_entry(addr, data);
}

bool Lemonsoc::run_till_pc(uint32_t pc) {
  int bound = 10000;
  int c = 0;
  while (get_pc() != pc && !Verilated::gotFinish()) {
    if (c >= bound)
      return false;
    if (!step())
      return false;
    c++;
  }
  return true;
}

uint32_t Lemonsoc::get_pc() {
  return tb->lemonsoc->lemon->pc_q;
}

uint32_t Lemonsoc::get_reg(uint8_t reg) {
  assert(reg < 32);
  auto regs = tb->lemonsoc->lemon->regfile->regs;
  return regs[reg];
}
