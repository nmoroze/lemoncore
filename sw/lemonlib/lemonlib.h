#ifndef LEMONLIB_H
#define LEMONLIB_H

#include <stdint.h>

#define LED1 0
#define LED2 1
#define LED3 2
#define LED4 3
#define LED5 4

#define BTN1 0
#define BTN2 1
#define BTN3 2

void delay(int ms);
void write_led(int led, int value);
void write_leds(int mask);
int read_button(int btn);
uint32_t read_timer();

#endif
