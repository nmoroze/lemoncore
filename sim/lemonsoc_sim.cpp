#include <stdlib.h>
#include <fstream>
#include <iostream>
#include "verilated.h"

#include "lemonsoc.h"

#ifndef FIRMWARE_PATH
#define FIRMWARE_PATH "../soc/sw/hello.sim.mem"
#endif

#define NUM_CYCLES -1

char led2c(bool led) {
  return led ? '*' : ' ';
}

void output_leds(bool leds[5]) {
  static bool first = true;
  if (!first) {
    for (int i = 0; i < 5; i++) {
      printf("\033[A");
      printf("\33[2K");
    }
  }
  printf("    (%c)     \n", led2c(leds[3]));
  printf("(%c) (%c) (%c) \n", led2c(leds[1]), led2c(leds[0]), led2c(leds[2]));
  printf("    (%c)     \n", led2c(leds[4]));
  first = false;
}

int main(int argc, char **argv) {
  Verilated::commandArgs(argc, argv);

  Lemonsoc soc(false, false);

  int cycle = 0;

  // Load test code
  if (!soc.load_firmware(FIRMWARE_PATH)) {
    std::cerr << "Error reading file " << FIRMWARE_PATH << std::endl;
    return 1;
  }

  while (!Verilated::gotFinish() && (NUM_CYCLES == -1 || cycle < NUM_CYCLES)) {
    soc.set_btns(0, 0, 0);

    if (!soc.step()) {
      printf("Error!\n");
      return 1;
    }

    if (soc.is_done()) {
      printf("Done!\n");
      return 0;
    }

    if (cycle % 99 == 0) {
      bool leds[5];
      for (int i = 0; i < 5; i++)
        leds[i] = (bool) soc.get_led(i+1);
      output_leds(leds);
    }

    cycle++;
  }

  return 0;
}
