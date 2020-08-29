#include "lemonlib/lemonlib.h"

#include <stdint.h>

// This program implements a binary counter on the 5 user LEDs. The speed of the
// count can be adjusted by pressing the 3 user buttons. Button 1 slows down the
// counter, button 3 speeds it up, and button 2 returns it to default speed.

int main() {
  int count = 0;
  int time = 50;
  int prev_btn1 = 0;
  int prev_btn2 = 0;
  int prev_btn3 = 0;

  uint32_t start = read_timer();

  while (1) {
    write_leds(count);
    // Increment count every 10 * time ms
    if (read_timer() - start > 10 * time) {
      count++;
      if (count > 31) count = 0;
      start = read_timer();
    }

    // Poll inputs
    int btn1 = read_button(BTN1);
    if (btn1 == 1 && prev_btn1 == 0) {
      time += 10;
      if (time > 300) time = 300;
    }
    prev_btn1 = btn1;

    int btn2 = read_button(BTN2);
    if (btn2 == 1 && prev_btn2 == 0) {
      time = 50;
    }
    prev_btn2 = btn2;

    int btn3 = read_button(BTN3);
    if (btn3 == 1 && prev_btn3 == 0) {
      time -= 10;
      if (time < 10) time = 10;
    }
    prev_btn3 = btn3;
  }
}
