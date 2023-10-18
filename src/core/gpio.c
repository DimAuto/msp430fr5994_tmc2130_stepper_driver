/**
 * @file gpio.c
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 0.1
 * @date 2022-05-20
 *
 *
 *
 */

#include "gpio.h"
#include "timer.h"

uint8_t gpio_initPin(Gpio_Pin *pin_t, uint8_t port, uint8_t pin, uint8_t mode) {
  // set pin mode
  pin_t->pin = pin;

  switch (port) {
    case 1:
      gpio_setMode(&P1DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P1IN;
        P1REN |= pin_t->pin;    //ENABLE PULLUP - PULLDOWN
        setBit(*(pin_t->port), pin_t->pin); //ENABLE PULLUP
      } else {
        pin_t->port = &P1OUT;
      }
      break;

    case 2:
      gpio_setMode(&P2DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P2IN;
        P2REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P2OUT;
      }
      break;

    case 3:
      gpio_setMode(&P3DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P3IN;
        P3REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P3OUT;
      }
      break;

    case 4:
      gpio_setMode(&P4DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P4IN;
        P4REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P4OUT;
      }
      break;

    case 5:
      gpio_setMode(&P5DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P5IN;
        P5REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P5OUT;
      }
      break;

    case 6:
      gpio_setMode(&P6DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P6IN;
        P6REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P6OUT;
      }
      break;

    case 7:
      gpio_setMode(&P7DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P7IN;
        P7REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P7OUT;
      }
      break;

    case 8:
      gpio_setMode(&P8DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P8IN;
        P8REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P8OUT;
      }
      break;

    case 9:
      gpio_setMode(&P9DIR, pin_t->pin, mode);
      if (mode == INPUT) {
        pin_t->port = &P9IN;
        P9REN |= pin_t->pin;
        setBit(*(pin_t->port), pin_t->pin);
      } else {
        pin_t->port = &P9OUT;
      }
      break;
  }
  return 0;
}

uint8_t gpio_readPin(Gpio_Pin *pin_t) {
  return getBit(*(pin_t->port), pin_t->pin) ? HIGH : LOW;
}

uint8_t gpio_writePin(Gpio_Pin *pin_t, uint8_t state) {
  if (state == HIGH) {
    setBit(*(pin_t->port), pin_t->pin);
  } else if (state == LOW) {
    clearBit(*(pin_t->port), pin_t->pin);
  }
  return 0;
}

void gpio_togglePin(Gpio_Pin *pin_t) { toggleBit(*(pin_t->port), pin_t->pin); }

uint8_t gpio_setMode(unsigned char *port, uint8_t pin, uint8_t mode) {
  if (mode == INPUT) {
    clearBit(*(port), pin);
  } else if (mode == OUTPUT) {
    setBit(*(port), pin);
  }

  else {
    return 1;
  }
  return 0;
}

uint8_t gpio_pattern(Gpio_Pin *pin_t) {
  uint8_t i = pin_t->pattern.repeat * 2;
  for (i; i > 1; i--) {
    gpio_togglePin(pin_t);
    delayMS(pin_t->pattern.delay_ms);
  }
  return 0;
}



