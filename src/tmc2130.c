/*
 * tmc2130.c
 *
 *  Created on: Aug 30, 2023
 *      Author: dkalaitzakis
 */


#include "core/timer.h"
#include "tmc2130.h"
#include "core/gpio.h"
#include "core/spi.h"

const uint32_t GCONF        = 0x00000001;
const uint32_t CHOPCONF     = 0x00010044;
const uint32_t TPOWERDOWN   = 0x00000040;
const uint32_t TPWMTHRS     = 0x000001F4;
const uint32_t PWM_CONF     = 0x000501FF;
const uint32_t TCOOLTHRS    = 0x00002700;
const uint32_t THIGH        = 0x00000300;
const uint32_t COOLCONF     = 0x01000000;
const uint32_t IHOLDIRUN    = 0x00101010;
const uint32_t MSTEPS       = 0x10008008;

static SPI_Mode tmc2130_spi_transfer(tmc2130_driver_t *driver, uint8_t addr, uint32_t data);


SPI_Mode tmc2130_init(tmc2130_driver_t *driver, uint8_t CS_port, uint8_t CS_pin, uint8_t EN_port, uint8_t EN_pin,
                      uint8_t DIR_port, uint8_t DIR_pin){
    SPI_Mode res;
    gpio_initPin(&(driver->DIR_Pin), DIR_port, DIR_pin, OUTPUT);
    gpio_initPin(&(driver->CS_Pin), CS_port, CS_pin, OUTPUT);
    gpio_initPin(&(driver->EN_Pin), EN_port, EN_pin, OUTPUT);
    gpio_writePin(&(driver->EN_Pin), HIGH);
    res = tmc2130_spi_transfer(driver, GCONF_ADDR, GCONF);
    delayMS(100);
    res = tmc2130_spi_transfer(driver, TCOOLTHRS_ADDR, TCOOLTHRS);
    delayMS(100);
    res = tmc2130_spi_transfer(driver, THIGH_ADDR, THIGH);
    delayMS(100);
    res = tmc2130_spi_transfer(driver, COOLCONF_ADDR, COOLCONF);
    delayMS(100);
    res = tmc2130_spi_transfer(driver, IHOLD_IRUN_ADDR, IHOLDIRUN);
    delayMS(100);
    res = tmc2130_spi_transfer(driver, CHOPCONF_ADDR, CHOPCONF);
    delayMS(100);
    res = tmc2130_spi_transfer(driver, PWMCONF_ADDR, PWM_CONF);
    delayMS(100);
    gpio_writePin(&(driver->EN_Pin), LOW);
    return res;
}

void tmc2130_disable(tmc2130_driver_t *driver){
    gpio_writePin(&(driver->EN_Pin), HIGH);
}

void tmc2130_enable(tmc2130_driver_t *driver){
    gpio_writePin(&(driver->EN_Pin), LOW);
}

SPI_Mode tmc2130_spi_transfer(tmc2130_driver_t *driver, uint8_t addr, uint32_t data){
    uint8_t data_8b[4] = {0};
    data_8b[3] = (data>>24) & 0xFF;
    data_8b[2] = (data>>16) & 0xFF;
    data_8b[1] = (data>>8)  & 0xFF;
    data_8b[0] = data & 0xFF;
    return SPI_Master_WriteReg(&(driver->CS_Pin) ,addr, data_8b, 4);
}

/**
 * Direction 0 =
 * @param dir
 */
void tmc2130_set_dir(tmc2130_driver_t *driver, uint8_t dir){
    if (dir == 0){
        gpio_writePin(&(driver->DIR_Pin), LOW);
    }
    else{
        gpio_writePin(&(driver->DIR_Pin), HIGH);
    }
}


void tmc2130_rotate_continuous(tmc2130_driver_t *driver, uint8_t dir, uint8_t dc, uint16_t speed){
    tmc2130_set_dir(driver, dir);
    float dtc = dc/50.0f;
    uint16_t duty = (uint16_t)(speed * dtc);
    pwm_set(speed*2, duty, 0);
}





void tmc2130_rotate_steps(tmc2130_driver_t *driver, uint16_t steps, uint16_t dir, uint16_t dc, uint16_t speed){
    tmc2130_set_dir(driver, dir);
    float dtc = dc/50.0f;
    uint16_t duty = (uint16_t)(speed * dtc);
    pwm_set(speed*2, duty, steps);
}
