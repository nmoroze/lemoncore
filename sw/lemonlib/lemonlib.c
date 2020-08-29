#include "lemonlib.h"

#include <stdint.h>

#define GPIO_BASE 0x3000

#ifdef SIM
#define ITERS_PER_MS 1
#else
#define ITERS_PER_MS 1500 // 1/1000 [sec/ms] * 12M [cycles/sec] * 1/8 [iters/cycle]
#endif

uint32_t timer = 0;

void delay(int time) {
  int iters = time * ITERS_PER_MS;
  // Inspired by AVR delay:
  // https://www.nongnu.org/avr-libc/user-manual/delay__basic_8h_source.html
  asm volatile("1: addi %0, %0, -1 \n\t"
               "bne x0, %0, 1b"
               : "=r" (iters)
               : "0" (iters));
}

// Even though GPIO peripheral uses a one-byte register, use uint32_t's to make
// sure memory accesses are word-aligned
void write_led(int led, int value) {
  if (led > LED5 || led < LED1)
    return;
  uint32_t val = *((uint32_t*) GPIO_BASE);
  val &= ~((uint32_t) (1 << led));
  val |= ((uint32_t) (value << led));
  *((uint32_t*) GPIO_BASE) = val;
}

void write_leds(int mask) {
  if (mask >= 1 << 5)
    return;

  uint32_t val = *((uint32_t*) GPIO_BASE);
  val &= ~((uint32_t) 0x1F);
  val |= (mask & 0x1F);
  *((uint32_t*) GPIO_BASE) = val;
}

int read_button(int button) {
  if (button > BTN3 || button < BTN1)
    return -1;

  return (*((uint32_t*) (GPIO_BASE + 0x8)) >> button) & 0x1;
}

uint32_t read_timer() {
  return timer;
}
