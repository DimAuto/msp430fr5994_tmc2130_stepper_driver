/**
 * @file timer.h
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 1.0
 * @date 2022-5-18
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include <msp430.h>
#include <stdint.h>

/**
 * Initializes the system clocks
 * ACLK = Auxiliary Clock
 * SMCLK = Subsystem Clock used by various peripherals.
 * MCLK = Master Clock used by the CPU
 */
void clock_init(void);
/**
 *Initializes the timer module.
 */
void timer_init(void);

/**
 * Blocking delay function.
 * @param msecs
 */
void delayMS(uint16_t msecs);

/**
 * Returns the current milli counter.
 */
uint16_t millis(void);

/**
 * Returns the ellapsed time in milliseconds
 * @return
 */
int ellapsed_millis(uint16_t st_timer);

/**
 * Enables PWM output on TA1 pin(1.2). Period and DC is configurable.
 * @param period
 * @param dutycycle
 */
void rot_pwm_set(uint16_t period, uint16_t dutycycle, uint16_t steps);

void inc_pwm_set(uint16_t period, uint16_t dutycycle, uint16_t steps);

/**
 * Stops the PWM output.
 */
void pwm_stop(void);

void watchdog_kick(void);

void watchdog_stop(void);


#endif /* SRC_TIMER_H_ */
