#include "lemoncore.h"

#include <stdint.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>

#include "Vlemoncore_lemoncore.h"
#include "Vlemoncore_regfile.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include "util.h"

#define DEFAULT_VCD_PATH "lemoncore.vcd"

Lemoncore::Lemoncore(bool verbose) {
  init(verbose, DEFAULT_VCD_PATH);
}

Lemoncore::Lemoncore(bool verbose, std::string vcd_path) {
  init(verbose, vcd_path);
}

void Lemoncore::init(bool verbose, std::string vcd_path) {
  this->verbose = verbose;
  tb = new Vlemoncore;
  cycle = 0;

  // Start tracing
  tfp = new VerilatedVcdC;
  Verilated::traceEverOn(true);
  tb->trace(tfp, 99);
  tfp->open(vcd_path.c_str());

  // Reset core
  reset();
}

Lemoncore::~Lemoncore() {
  // Stop tracing
  tfp->close();

  delete tb;
  delete tfp;
}

bool Lemoncore::load_firmware(std::string path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file) {
    return false;
  }
  log("Firmware contents: \n");
  for (int i = 0; i < (ROM_SIZE + RAM_SIZE) / 4 && file; i++) {
    file.read((char*)&mem[i], 4);
    log("0x%08x: 0x%08x\n", i * 4, mem[i]);
  }
  return true;
}

void Lemoncore::reset() {
  // Reset the core
  tb->rst_i = 1;
  tb->clk_i = 0;
  tb->eval();
  tfp->dump(cycle);
  tb->clk_i = 1;
  tb->eval();
  tb->rst_i = 0;
  tfp->dump(cycle + 1);

  cycle++;
}

void Lemoncore::dump_regs() {
  auto regs = tb->lemoncore->regfile->regs;
  for (int i = 0; i < 32; i++) {
    // TODO: definitely better way to do alignment
    char* align = "";
    if (i < 10) align = " ";
    printf("%sr%d: 0x%08x\n", align, i, regs[i]);
  }
}

bool Lemoncore::step() {
  tb->clk_i = 0;
  tb->eval();
  tfp->dump(2 * cycle);
  tb->clk_i = 1;
  tb->eval();
  tfp->dump(2 * cycle + 1);

  cycle++;

  tb->instr_res_valid_i = 0;
  tb->mem_read_res_valid_i = 0;
  tb->mem_write_res_valid_i = 0;

  // Requesting instruction memory
  if (tb->instr_req_valid_o) {
    uint32_t addr = tb->instr_req_addr_o;
    log("Requesting instruction @ 0x%08x\n", addr);

    assert(addr % 4 == 0);
    assert(addr < ROM_SIZE);

    uint32_t data = mem[addr / 4];
    log("Instruction: ");
    if (verbose) print_instruction(data);

    tb->instr_res_valid_i = 1;
    tb->instr_res_data_i = data;
  }

  // Data memory read
  if (tb->mem_read_req_valid_o) {
    uint32_t addr = tb->mem_read_req_addr_o;
    log("Reading memory @ 0x%08x\n", addr);

    // TODO: should support partial loads
    assert(addr % 4 == 0);
    if (addr < ROM_SIZE || addr >= ROM_SIZE + RAM_SIZE) {
      std::cout << "Address out of bounds: " << addr << std::endl;
      dump_regs();
      assert(false);
    }

    uint32_t data = mem[addr / 4];
    log("Response: 0x%08x\n", data);

    tb->mem_read_res_valid_i = 1;
    tb->mem_read_res_data_i = data;
  }

  // Data memory write
  if (tb->mem_write_req_valid_o) {
    uint32_t addr = tb->mem_write_req_addr_o;
    uint32_t data = tb->mem_write_req_data_o;
    log("Writing value 0x%08x to location 0x%08x\n", data, addr);

    // TODO: should support partial stores
    assert(addr % 4 == 0);
    if (addr < ROM_SIZE || addr >= ROM_SIZE + RAM_SIZE) {
      std::cout << "Address out of bounds: " << addr << std::endl;
      dump_regs();
      assert(false);
    }

    tb->mem_write_res_valid_i = 1;
    mem[addr / 4] = data;
  }

  return true;
}

bool Lemoncore::run(int cycles) {
  for (int c = 0; c < cycles && !Verilated::gotFinish(); c++) {
    if (!step())
      return false;
  }

  return true;
}

bool Lemoncore::run_till_pc(uint32_t pc) {
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

void Lemoncore::log(const char* fmt...) {
  // https://stackoverflow.com/q/41400
  if (verbose) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
}

void Lemoncore::write_imem(uint32_t addr, uint32_t data) {
  assert(addr % 4 == 0);
  assert(addr < ROM_SIZE);
  mem[addr / 4] = data;
}

void Lemoncore::write_ram(uint32_t addr, uint32_t data) {
  assert(addr % 4 == 0);
  assert(addr < RAM_SIZE + ROM_SIZE);
  mem[(addr + ROM_SIZE) / 4] = data;
}

uint32_t Lemoncore::read_ram(uint32_t addr) {
  assert(addr % 4 == 0);
  assert(addr < RAM_SIZE + ROM_SIZE);
  return mem[(addr + ROM_SIZE) / 4];
}

void Lemoncore::set_reg(uint8_t reg, uint32_t data) {
  tb->lemoncore->regfile->regs[reg] = data;
}

uint32_t Lemoncore::get_reg(uint8_t reg) {
  assert(reg < 32);
  auto regs = tb->lemoncore->regfile->regs;
  return regs[reg];
}

uint32_t Lemoncore::get_pc() {
  return tb->lemoncore->pc_q;
}

uint32_t Lemoncore::get_mcause() {
  return tb->lemoncore->mcause_q;
}

uint32_t Lemoncore::get_mstatus() {
  return (tb->lemoncore->mstatus_mie << 3) |  (tb->lemoncore->mstatus_mpie << 7);
}

void Lemoncore::set_mstatus(uint32_t mstatus) {
  tb->lemoncore->mstatus_mie = (mstatus >> 3) & 0x1;
  tb->lemoncore->mstatus_mpie = (mstatus >> 7) & 0x1;
}


uint32_t Lemoncore::get_mtval() {
  return tb->lemoncore->mtval_q;
}

uint32_t Lemoncore::get_mscratch() {
  return tb->lemoncore->mscratch_q;
}

uint32_t Lemoncore::get_mip() {
  return tb->lemoncore->mip_external << 11 | tb->lemoncore->mip_timer << 7 | tb->lemoncore->mip_software << 3;
}

void Lemoncore::set_mie(uint32_t mie) {
  tb->lemoncore->mie_external = (mie >> 11) & 0x1;
  tb->lemoncore->mie_timer = (mie >> 7) & 0x1;
  tb->lemoncore->mie_software = (mie >> 3) & 0x1;
}

void Lemoncore::set_irq_timer(int val) {
  tb->irq_timer_i = val;
}

void Lemoncore::set_irq_software(int val) {
  tb->irq_software_i = val;
}

void Lemoncore::set_irq_external(int val) {
  tb->irq_external_i = val;
}
