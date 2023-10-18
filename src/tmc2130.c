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
#include "core/uart.h"
#include "message_handler.h"

const uint32_t GCONF        = 0x00000001;
const uint32_t CHOPCONF     = 0x00010044;
const uint32_t TPOWERDOWN   = 0x00000040;
const uint32_t TPWMTHRS     = 0x000001F4;
const uint32_t PWM_CONF     = 0x000501FF;
const uint32_t TCOOLTHRS    = 0x00002700;
const uint32_t THIGH        = 0x00000300;
const uint32_t COOLCONF     = 0x01000000;
const uint32_t IHOLDIRUN    = 0x00161616;
const uint32_t MSTEPS       = 0x00008008;

static SPI_Mode tmc2130_spi_transfer(tmc2130_driver_t *driver, uint8_t addr, uint32_t data);
static void send_compl_ack(void);
static void send_compl_nack(uint16_t steps);
static uint8_t moving_flag = 0;


void tick_TerminalSW(tmc2130_driver_t *driver){
    switch(driver->StpSW_state.state){
    case PUSHED:
        if(ellapsed_millis(driver->StpSW_state.timer) > driver->StpSW_state.debounce_time){
            if(!gpio_readPin(&driver->Stop_SW)){
                driver->StpSW_state.state = HOLD;
                gpio_writePin(&driver->StpSW_state.led, HIGH);
            }
            else{
                driver->StpSW_state.state = RELEASED;
                gpio_writePin(&driver->StpSW_state.led, LOW);
            }
        }
        break;
    case RELEASED:
        if(!gpio_readPin(&driver->Stop_SW)){
            driver->StpSW_state.timer = millis();
            driver->StpSW_state.state = PUSHED;
            pwm_stop();
            send_compl_nack(get_remain_steps);
            moving_flag = 0;
        }
        break;
    case HOLD:
        if(!gpio_readPin(&driver->Stop_SW)){
            driver->StpSW_state.timer = millis();
        }
        else{
            if(ellapsed_millis(driver->StpSW_state.timer) > driver->StpSW_state.debounce_time){
                if(gpio_readPin(&driver->Stop_SW)){
                    driver->StpSW_state.state = RELEASED;
                    gpio_writePin(&driver->StpSW_state.led, LOW);
                }
            }
        }
        break;
    }
    if(moving_flag == 1){
        if(get_compl_flag()){
            send_compl_ack();
            moving_flag = 0;
        }
    }
}

SPI_Mode tmc2130_init(tmc2130_driver_t *driver, uint8_t CS_port, uint8_t CS_pin, uint8_t EN_port, uint8_t EN_pin,
                      uint8_t DIR_port, uint8_t DIR_pin, uint8_t StpSW_port, uint8_t StpSW_pin){
    SPI_Mode res;

    gpio_initPin(&driver->CS_Pin, CS_port, CS_pin, OUTPUT);
    gpio_initPin(&driver->EN_Pin, EN_port, EN_pin, OUTPUT);
    gpio_initPin(&driver->DIR_Pin, DIR_port, DIR_pin, OUTPUT);
    gpio_initPin(&driver->Stop_SW, StpSW_port, StpSW_pin, INPUT);

    gpio_initPin(&driver->StpSW_state.led, 1, BIT1, OUTPUT);
    gpio_writePin(&driver->StpSW_state.led, LOW);

    driver->StpSW_state.state = RELEASED;
    driver->StpSW_state.debounce_time = 30;

    gpio_writePin(&driver->EN_Pin, HIGH);
    delayMS(100);
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
//    delayMS(100);
//    res = tmc2130_spi_transfer(driver, PWMCONF_ADDR, PWM_CONF);
//    delayMS(200);
//    gpio_writePin(&driver->EN_Pin, LOW);

    return res;
}

void send_compl_ack(void){
    uint8_t data[3] = {0};
    data[0] = ACK;
    transmitMessage(data, 3, 0xC5, UART_NYX);
}

void send_compl_nack(uint16_t steps){
    uint8_t data[3] = {0};
    data[0] = NACK;
    data[1] = steps & 0xFF;
    data[2] = (steps >> 8) & 0xFF;
    transmitMessage(data, 3, 0xC5, UART_NYX);
}

void tmc2130_disable(tmc2130_driver_t *driver){
    gpio_writePin(&driver->EN_Pin, HIGH);
}

void tmc2130_enable(tmc2130_driver_t *driver){
    gpio_writePin(&driver->EN_Pin, LOW);
}

SPI_Mode tmc2130_spi_transfer(tmc2130_driver_t *driver, uint8_t addr, uint32_t data){
    uint8_t data_8b[4] = {0};
    data_8b[0] = (data>>24) & 0xFF;
    data_8b[1] = (data>>16) & 0xFF;
    data_8b[2] = (data>>8)  & 0xFF;
    data_8b[3] = data & 0xFF;
    return SPI_Master_WriteReg(&driver->CS_Pin ,addr, data_8b, 4);
}

/**
 * Direction 0 =
 * @param dir
 */
void tmc2130_set_dir(tmc2130_driver_t *driver, uint8_t dir){
    if (dir == 0){
        gpio_writePin(&driver->DIR_Pin, LOW);
    }
    else{
        gpio_writePin(&driver->DIR_Pin, HIGH);
    }
}


void tmc2130_rotate_continuous(tmc2130_driver_t *driver, uint8_t dir, uint8_t dc, uint16_t speed){
    tmc2130_set_dir(driver, dir);
    float dtc = dc/50.0f;
    uint16_t duty = (uint16_t)(speed * dtc);
    pwm_set(speed*2, duty, 0);
}


void tmc2130_rotate_steps(tmc2130_driver_t *driver, uint16_t steps, uint16_t dir, uint16_t dc, uint16_t speed, motor_select_t motor){
    tmc2130_set_dir(driver, dir);
    float dtc = dc/50.0f;
    uint16_t duty = (uint16_t)(speed * dtc);
    if(motor == ROT_MOTOR){
        if((driver->StpSW_state.state == HOLD) && (dir != INC_RELEASE_DIR)){
            pwm_stop();
        }
        else{
            rot_pwm_set(speed*2, duty, steps);
        }
    }
    else{
        if((driver->StpSW_state.state == HOLD) && (dir != INC_RELEASE_DIR)){
            pwm_stop();
        }
        else{
            inc_pwm_set(speed*2, duty, steps);
        }
    }
    moving_flag = 1;
}


uint8_t get_stop_switch(tmc2130_driver_t *driver){
    if(driver->StpSW_state.state == HOLD){
        return 1;
    }
    return 0;
}

void goto_start_position(tmc2130_driver_t *driver, motor_select_t motor){
    tmc2130_rotate_steps(driver, 10000, !INC_RELEASE_DIR, 32, 300, motor);
    uint16_t timeout = millis();
    while(1){
        tick_TerminalSW(driver);
        if(driver->StpSW_state.state == HOLD){
            break;
        }
        if(ellapsed_millis(timeout) > 4000){
            break;
        }
    }
    tmc2130_rotate_steps(driver, 4266, INC_RELEASE_DIR, 32, 300, motor);
    timeout = millis();
    while(1){
        if(ellapsed_millis(timeout) > 4000){
            break;
        }
        if(get_compl_flag()){
            break;
        }
    }
//    tmc2130_disable(driver);
}

//// Port 8 interrupt service routine
//#pragma vector=PORT8_VECTOR
//__interrupt void Port_8(void)
//{
//    switch(__even_in_range(P8IV,16))
//    {
//    case 0: break; // No Interrupt
//    case 2: break; // P8.0
//    case 4: break;
//    case 6:
//        rot_StpSW_flag = 1;
//        break;
//    case 8:
//        inc_StpSW_flag = 1;
//        break;
//    case 10: break;
//    case 12: break;
//    case 14: break;
//    case 16: break;
//    }
//}
