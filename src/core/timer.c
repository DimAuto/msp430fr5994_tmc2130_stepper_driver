/**
 * @file timer.c
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 1.0
 * @date 2022-05-18
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "timer.h"
#include "src/config.h"

static volatile uint16_t counter;
static volatile uint16_t ellapsed_mill = 0;
static volatile uint16_t pwm_counter = 0;
static volatile uint8_t rot_completion = 0;
static uint16_t remain_steps = 0;
static uint16_t step_count = 0;
static uint16_t speed_sel = 300;

void clock_init(void){
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL;           // Set DCO to 16MHz
    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0;                           // Lock CS registersock CS registers
    P1SEL0 |= BIT2 | BIT3; // Set P1.2 pin for PWM output.
    P1DIR |= BIT2 | BIT3;
    _enable_interrupt();
}

void timer_init(void) {
  TB0CCTL0 |= CCIE;
  TB0CTL = TBSSEL_2 + ID_3 + MC_1;  // Select SMCLK, SMCLK/1, Up Mode

  TA2CCTL0 |= CCIE;
  TA2CTL = TASSEL_2 + ID_3 + MC_1;
  TA2CCR0 = 2000 - 1;
}

void rot_pwm_set(uint16_t period, uint16_t dutycycle, uint16_t steps){
    rot_completion = 0;
    step_count = steps;
    speed_sel = period;
    TA1CCR0 = 600;                       // PWM Period
    TA1CCTL1 = OUTMOD_7;                    // CCR2 reset/set
    TA1CCR1 = dutycycle;                          // CCR2 PWM duty cycle
    TA1CCTL2 = OUTMOD_5;
    TA1CCR2 = 0;
    TA1CCTL0 |= CCIE;
    TA1CTL = TASSEL__SMCLK + MC__UP + ID_3 + TACLR;// SMCLK, up mode, clear TAR
}

void inc_pwm_set(uint16_t period, uint16_t dutycycle, uint16_t steps){
    rot_completion = 0;
    step_count = steps;
    speed_sel = period;
    TA1CCR0 = 600;                       // PWM Period
    TA1CCTL2 = OUTMOD_7;                    // CCR2 reset/set
    TA1CCR2 = dutycycle;
    TA1CCTL1 = OUTMOD_5; // CCR2 PWM duty cycle
    TA1CCR1 = 0;
    TA1CCTL0 |= CCIE;
    TA1CTL = TASSEL__SMCLK + MC__UP + ID_3 + TACLR;// SMCLK, up mode, clear TAR
}

void pwm_stop(void){
    TA1CTL = MC__STOP;
    remain_steps = step_count - pwm_counter;
    rot_completion = 0;
    pwm_counter = 0;
}

uint8_t get_compl_flag(void){
    return rot_completion;
}

uint16_t get_remain_steps(void){
    return remain_steps;
}

void timer_deinit(void) {
  TA2CCR0 = 0;
  TB0CCR0 = 0;
}

void delayMS(uint16_t msecs) {
  counter = 0;  // Reset Over-Flow counter
  TB0CCR0 =
      2000 -
      1;  // Start Timer, Compare value for Up Mode to get 1ms delay per loop

  while (counter <= msecs)
    ;
  TB0CCR0 = 0;  // Stop Timer
}

void watchdog_stop(void){
    WDTCTL = WDTPW | WDTHOLD;
}

void watchdog_kick(void){
    WDTCTL = WDT_ARST_1000;
}

uint16_t millis(void) { return ellapsed_mill; }

int ellapsed_millis(uint16_t st_timer) {
  int tmp = (ellapsed_mill - st_timer);
  if (tmp < 0) {
    tmp = (UINT16_T_MAX_VALUE - tmp);
    return tmp;
  }
  return tmp;
}

#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR(void) { counter++; }


#pragma vector = TIMER2_A0_VECTOR
__interrupt void TIMER2_A0_ISR(void) {
  ellapsed_mill++;
  if (ellapsed_mill > UINT16_T_MAX_VALUE - 1) {
    ellapsed_mill = 0;
  }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0_ISR(void){
    pwm_counter++;
    if ((pwm_counter >= step_count) && (step_count != 0)){
        TA1CTL = MC__STOP;  // Stop PWM
        pwm_counter = 0;
        rot_completion = 1;
    }
    if ((pwm_counter >= 512) && (pwm_counter <= step_count)){
        TA1CCR0 = speed_sel;
    }
    else if ((pwm_counter >= step_count - 512) && (pwm_counter <= step_count)){
        TA1CCR0 = 600;
    }
}


