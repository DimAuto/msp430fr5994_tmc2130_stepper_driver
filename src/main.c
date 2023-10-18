/*
 * main.c
 *
 *  Created on: Aug 30, 2023
 *      Author: dkalaitzakis
 */
#include <stdint.h>
#include "config.h"
#include "message_handler.h"
#include "core/uart.h"
#include "tmc2130.h"
#include "core/gpio.h"

tmc2130_driver_t rot_motor, inc_motor;


static uint8_t get_connected_flag(void);

static void system_init(void);


void main(void) {

    system_init();
    while(1){

        tick_Handler();
        tick_TerminalSW(&inc_motor);

    }
}


void system_init(void){
    watchdog_stop();
    clock_init();
    timer_init();
    PMM_unlockLPM5();
    delayMS(200);


    uart_init(UART_NYX);

    initSPI();

    delayMS(200);
    tmc2130_init(&rot_motor, 5, BIT3, 4, BIT1, 4, BIT2, 8, BIT2);
    tmc2130_init(&inc_motor, 2, BIT6, 8, BIT1, 4, BIT3, 8, BIT3);


    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings

}
