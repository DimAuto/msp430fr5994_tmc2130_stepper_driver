/*
 * gpio.h
 *
 *  Created on: May 19, 2022
 *      Author: dkalaitzakis
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

#include <stdint.h>

#include "msp430.h"

#define HIGH 1
#define LOW 0
#define INPUT 0x00
#define OUTPUT 0x01

#define setBit(port, pin) ((port) |= (pin))
#define clearBit(port, pin) ((port) &= ~(pin))
#define toggleBit(port, pin) ((port) ^= (pin))
#define getBit(port, pin) (((port) & (pin)) != 0)

typedef struct {
  uint16_t delay_ms;
  uint16_t repeat;
} Gpio_Pattern_t;

typedef struct {
  volatile unsigned char *port;
  uint8_t pin;
  uint8_t mode;
  uint8_t state;
  Gpio_Pattern_t pattern;
} Gpio_Pin;

/**
 * A function to initialize a gpio pin.
 * @param pin
 * @param port
 * @param pin_t
 * @param mode
 */
uint8_t gpio_initPin(Gpio_Pin *pin_t, uint8_t port, uint8_t pin, uint8_t mode);

/**
 *
 * @param pin_t
 * @return
 */
uint8_t gpio_readPin(Gpio_Pin *pin_t);

/**
 *
 * @param port
 * @param pin_t
 * @param mode
 */
uint8_t gpio_setMode(unsigned char *port, uint8_t pin, uint8_t mode);

/**
 * A function to write value on a gpio pin.
 * @param pin_t
 */
uint8_t gpio_writePin(Gpio_Pin *pin_t, uint8_t state);

/**
 *
 * @param pin_t
 */
void gpio_togglePin(Gpio_Pin *pin_t);

/**
 *
 * @param pin_t
 * @return
 */
uint8_t gpio_pattern(Gpio_Pin *pin_t);

#endif /* SRC_GPIO_H_ */
