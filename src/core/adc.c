/*
 * adc.c
 *
 *  Created on: Jun 6, 2022
 *      Author: dkalaitzakis
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "adc.h"
#include "timer.h"
#include "uart.h"
#include "helpers.h"
#include <msp430.h>

ADC_val_t adc_values;


void tick_adc(void){
    uint8_t i;
    adc_values.adc0_filtered = adc_read(0);        //VOLTAGE VALUE *=8
    adc_values.adc1_filtered = adc_read(1);        //   *=1
    adc_values.adc3_filtered = adc_read(3);        //VOLTAGE VALUE *=8
#ifdef __DEBUG__
    uint8_t msg[10]={0};
    itoa(adc_values.adc0_filtered, msg, 10);
    uart_write_DEBUG(" P1.0=",UART_DEBUG);
    uart_write_DEBUG(msg, UART_DEBUG);
    memset(msg, 0, 10);
    itoa(adc_values.adc1_filtered, msg, 10);
    uart_write_DEBUG(" P1.1=",UART_DEBUG);
    uart_write_DEBUG(msg, UART_DEBUG);
    memset(msg, 0, 10);
    itoa(adc_values.adc3_filtered, msg, 10);
    uart_write_DEBUG(" P1.3=",UART_DEBUG);
    uart_write_DEBUG(msg, UART_DEBUG);
    uart_write_DEBUG("\r\n",UART_DEBUG);
    memset(msg, 0, 10);
#endif

}

uint8_t adc_init(void) {
  // Configure ADC Pins
  P1SEL1 |= (BIT0 | BIT1 | BIT3); //Configure pin 1.0-1.1-1.3 as ADC inputs
  P1SEL0 |= (BIT0 | BIT1 | BIT3);

  adc_values.adc_val0 = 0;
  adc_values.adc_val1 = 0;
  adc_values.adc_val2 = 0;
  adc_values.adc_val3 = 0;

  PM5CTL0 &= ~LOCKLPM5;
  // Configure ADC12
  //  Turn on ADC and enable multiple conversions
  ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_4;
  ADC12CTL1 |= ADC12SHP + ADC12CONSEQ_1;
  ADC12MCTL0 = ADC12INCH_0;
  ADC12MCTL1 = ADC12INCH_1;
  ADC12MCTL2 = ADC12INCH_2;
  ADC12MCTL3 = ADC12INCH_3 + ADC12EOS;  //+end of sequence
  ADC12IER0 |= ADC12IE3;

  return 0;
}

void adc_conv_start(void) {
  ADC12IER0 |= ADC12IE3;
  ADC12CTL0 |= ADC12ENC | ADC12SC;
}

void adc_conv_stop(void) {
  ADC12CTL0 &= ~(ADC12ENC | ADC12SC);
  ADC12IER0 &= ~ADC12IE3;
}

uint16_t adc_read(uint8_t channel) {
  uint16_t sum;
  uint8_t i;
  memset(adc_values.filter_buffer, 0, FILTER_LEN);
  // Block until the final channel is loaded with value.
  for (i = 0; i < FILTER_LEN; i++) {
    adc_conv_start();
    delayMS(5);
    while (ADC12IFGR0 & ADC12IFG3) {
      ;
    }
    switch (channel) {
      case 0:
          adc_values.filter_buffer[i] = adc_values.adc_val0;
        break;

      case 1:
          adc_values.filter_buffer[i] = adc_values.adc_val1;
        break;

      case 2:
          adc_values.filter_buffer[i] = adc_values.adc_val2;
          break;

      case 3:
          adc_values.filter_buffer[i] = adc_values.adc_val3;
        break;
    }
    adc_conv_stop();
    delayMS(5);
  }

  for (i = 0; i < FILTER_LEN; i++) {
    sum += adc_values.filter_buffer[i];
  }
  return sum / FILTER_LEN;
}

float adc_conv(uint16_t adc_result) {
  float result = (adc_result * (VREF / ADC12_RES));
  return result;
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_A_ISR(void) {
  switch (__even_in_range(ADC12IV, ADC12IV__ADC12RDYIFG)) {
  case ADC12IV__ADC12IFG0:   break;         // Vector 12:  ADC12MEM0 Interrupt
  case ADC12IV__ADC12IFG1:   break;   // Vector 14:  ADC12MEM1
  case ADC12IV__ADC12IFG2:   // Vector 16:  ADC12MEM2
      break;
  case ADC12IV__ADC12IFG3:   // Vector 18:  ADC12MEM3
      ADC12CTL0 &= ~ADC12SC;
      adc_values.adc_val0 = ADC12MEM0;
      adc_values.adc_val1 = ADC12MEM1;
      adc_values.adc_val3 = ADC12MEM2;
      adc_values.adc_val3 = ADC12MEM3;
      __bic_SR_register_on_exit(LPM0_bits);
      break;
  default:
      break;
  }
}
