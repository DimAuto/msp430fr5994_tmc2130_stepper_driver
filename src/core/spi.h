/*
 * spi.h
 *
 *  Created on: Aug 30, 2023
 *      Author: dkalaitzakis
 */

#ifndef SRC_CORE_SPI_H_
#define SRC_CORE_SPI_H_

#include <stdint.h>
#include "gpio.h"

//******************************************************************************
// Pin Config ******************************************************************
//******************************************************************************

#define WRITE_FLAG (1<<7)
#define READ_FLAG (0<<7)

#define DUMMY   0xFF

#define MAX_BUFFER_SIZE     20

typedef enum SPI_ModeEnum{
    IDLE_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    TIMEOUT_MODE,
    SPI_ERROR
} SPI_Mode;

void initSPI(void);

SPI_Mode SPI_Master_WriteReg(Gpio_Pin *CS_Pin, uint8_t reg_addr, uint8_t *reg_data, uint8_t count);

SPI_Mode SPI_Master_ReadReg(Gpio_Pin *CS_Pin, uint8_t reg_addr, uint8_t count);

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);

void SendUCB1Data(uint8_t val);



#endif /* SRC_CORE_SPI_H_ */
