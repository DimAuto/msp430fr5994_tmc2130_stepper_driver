/*
 * tmc2130.h
 *
 *  Created on: Aug 30, 2023
 *      Author: dkalaitzakis
 */

#ifndef SRC_TMC2130_H_
#define SRC_TMC2130_H_

#include <stdint.h>
#include "core/spi.h"
#include "core/gpio.h"

//#define SLAVE_CS_OUT    P5OUT
//#define SLAVE_CS_DIR    P5DIR
//#define SLAVE_CS_PIN    BIT3
//
//#define SLAVE_RST_OUT   P1OUT
//#define SLAVE_RST_DIR   P1DIR
//#define SLAVE_RST_PIN   BIT4

#define GCONF_ADDR      0x00
#define GSTAT_ADDR      0x01
#define IHOLD_IRUN_ADDR 0x10
#define TPOWERDOWN_ADDR 0x11
#define TSTEP_ADDR      0x12
#define TPWMTHRS_ADDR   0x13
#define TCOOLTHRS_ADDR  0x14
#define THIGH_ADDR      0x15
#define MSLUT_ADDR      0x60
#define CHOPCONF_ADDR   0x6C
#define COOLCONF_ADDR   0x6D
#define PWMCONF_ADDR    0x70


typedef struct{
    Gpio_Pin CS_Pin;
    Gpio_Pin EN_Pin;
    Gpio_Pin DIR_Pin;
}tmc2130_driver_t;

typedef enum{
    ROT_MOTOR,
    INC_MOTOR
}motor_select_t;

SPI_Mode tmc2130_init(tmc2130_driver_t *driver, uint8_t CS_port, uint8_t CS_pin, uint8_t EN_port, uint8_t EN_pin,
                      uint8_t DIR_port, uint8_t DIR_pin);

//SPI_Mode tmc2130_init(tmc2130_driver_t *driver);

void tmc2130_set_dir(tmc2130_driver_t *driver, uint8_t dir);

void tmc2130_rotate_continuous(tmc2130_driver_t *driver, uint8_t dir, uint8_t dc, uint16_t speed);

void tmc2130_rotate_steps(tmc2130_driver_t *driver, uint16_t steps, uint16_t dir, uint16_t dc, uint16_t speed, motor_select_t motor);

void tmc2130_enable(tmc2130_driver_t *driver);

void tmc2130_disable(tmc2130_driver_t *driver);

#endif /* SRC_TMC2130_H_ */
