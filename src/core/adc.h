/**
 * @file adc.h
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 1.0
 * @date 2022-06-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SRC_CORE_ADC_H_
#define SRC_CORE_ADC_H_

#include <stdint.h>

#define VREF        3.3
#define ADC12_RES   4095
#define ADC10RES    1023
#define FILTER_LEN  4

typedef struct{
    uint16_t filter_buffer[FILTER_LEN];
    volatile uint16_t adc_val0;
    volatile uint16_t adc_val1;
    volatile uint16_t adc_val2;
    volatile uint16_t adc_val3;
    uint16_t adc0_filtered;
    uint16_t adc1_filtered;
    uint16_t adc2_filtered;
    uint16_t adc3_filtered;

}ADC_val_t;

/**
 * Ticks all adc conversions and saves the data.
 */
void tick_adc(void);


/**
 * ADC initialization function
 */
uint8_t adc_init(void);

/**
 * Start Conversion
 */
void adc_conv_start(void);

/**
 * Stop conversion
 */
void adc_conv_stop(void);

/**
 * Blocking fucntion which reads a specific channel. The function uses a moving average filter.
 * @param result
 * @param channel
 */
uint16_t adc_read(uint8_t channel);

/**
 * Converts the adc read value to voltage
 */
float adc_conv(uint16_t adc_result);

#endif /* SRC_CORE_ADC_H_ */
