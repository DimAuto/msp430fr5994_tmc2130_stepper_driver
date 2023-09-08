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

tmc2130_driver_t rot_motor, inc_motor;

static void system_init(void);


void main(void) {
    system_init();
    while(1){

        tick_Handler();
    }
}


void system_init(void){
    watchdog_stop();
    // Set P1.0 to output direction
    clock_init();
    timer_init();
    delayMS(200);


    uart_init(UART_NYX);

    initSPI();

    adc_init();
    delayMS(200);
    tmc2130_init(&rot_motor, 5, BIT3, 4, BIT1, 4, BIT2);
    tmc2130_init(&inc_motor, 2, BIT6, 8, BIT1, 4, BIT3);
    uart_write_DEBUG("Initialized!\r\n", UART_NYX);

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
}
