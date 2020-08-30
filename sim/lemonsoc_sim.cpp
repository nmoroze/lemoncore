#include <stdlib.h>
#include <fstream>
#include <iostream>

#include <cxxopts.hpp>
#include <ncurses.h>

#include "verilated.h"

#include "lemonsoc.h"

#define DEFAULT_FW_PATH "sw/hello.sim.mem"
#define NUM_CYCLES -1

bool btn1 = false;
bool btn2 = false;
bool btn3 = false;

char led2c(bool led) {
  return led ? '*' : ' ';
}

void output_leds(bool leds[5]) {
  move(0, 0);
  printw("    [%c]     \n", led2c(leds[3]));
  printw("[%c] [%c] [%c] \n", led2c(leds[1]), led2c(leds[0]), led2c(leds[2]));
  printw("    [%c]     \n", led2c(leds[4]));
  printw("\n");
  printw("Button 1: %s\n", btn1 ? "pressed" : "released");
  printw("Button 2: %s\n", btn2 ? "pressed" : "released");
  printw("Button 3: %s\n", btn3 ? "pressed" : "released");
  printw("\n");
  printw("Press keyboard keys 1 - 3 to toggle buttons. Press q to exit.\n");
  refresh();
}

int run (std::string firmware_path) {
  Lemonsoc soc(false, false);

  int cycle = 0;

  // Load test code
  if (!soc.load_firmware(firmware_path)) {
    std::cerr << "Error reading file " << firmware_path << std::endl;
    return 1;
  }

  while (!Verilated::gotFinish() && (NUM_CYCLES == -1 || cycle < NUM_CYCLES)) {
    char input = getch();
    if (input == '1')
      btn1 = !btn1;
    else if (input == '2')
      btn2 = !btn2;
    else if (input == '3')
      btn3 = !btn3;
    else if (input == 'q')
      return 0;
    soc.set_btns(btn1, btn2, btn3);

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

int main(int argc, char **argv) {
  Verilated::commandArgs(argc, argv);

  // Parse command line options
  cxxopts::Options options("socsim", "Interactive simulation of Lemoncore SoC");
  options.add_options()
    ("f,firmware", "Path to firmware file", cxxopts::value<std::string>()->default_value(DEFAULT_FW_PATH))
    ("h,help", "Print usage")
    ;

  std::string firmware_path;
  try {
    auto result = options.parse(argc, argv);
    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
    }
    firmware_path = result["firmware"].as<std::string>();
  } catch (cxxopts::OptionException e) {
    std::cerr << "Error parsing command line arguments: " << e.what() << std::endl;
    return 1;
  }


  // Init ncurses
  initscr();
  nodelay(stdscr, TRUE); // don't block on getch()
  noecho(); // don't echo input

  // Run simulation
  int r = run(firmware_path);

  // De-init ncurses
  endwin();

  return r;
}
