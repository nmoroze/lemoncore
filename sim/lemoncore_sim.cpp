#include <stdlib.h>
#include <iostream>

#include "lemoncore.h"
#include "verilated.h"

// Simulation settings and parameters
// TODO: set from command line
#define FIRMWARE_PATH "sw/hello.bin"
#define NUM_CYCLES 100

#define VERBOSE true

int main(int argc, char **argv) {
  // Initialize Verilators variables
  Verilated::commandArgs(argc, argv);

  Lemoncore cpu(VERBOSE);

  // Load test code
  if (!cpu.load_firmware(FIRMWARE_PATH)) {
    std::cerr << "Error reading file " << FIRMWARE_PATH << std::endl;
    exit(EXIT_FAILURE);
  }

  // Run CPU
  if (cpu.run(NUM_CYCLES)) {
    exit(EXIT_SUCCESS);
  }
  else {
    exit(EXIT_FAILURE);
  }
}
