#include <stdlib.h>
#include <iostream>

#include "lemoncore.h"
#include "verilated.h"

// Simulation settings and parameters
// TODO: set from command line
#define NUM_CYCLES 100

#define VERBOSE true

int main(int argc, char **argv) {
  // Initialize Verilators variables
  Verilated::commandArgs(argc, argv);

  // Get firmware path
  const char* flag_firmware = Verilated::commandArgsPlusMatch("firmware");
  std::string firmware_path;
  if (flag_firmware[0]) {
    firmware_path = std::string(flag_firmware + strlen("+firmware="));
  } else {
    std::cerr << "Please specify path to firmware file!" << std::endl;
    exit(EXIT_FAILURE);
  }

  Lemoncore cpu(VERBOSE);

  // Load test code
  if (!cpu.load_firmware(firmware_path)) {
    std::cerr << "Error reading file " << firmware_path << std::endl;
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
