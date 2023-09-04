/**
 * @file i2c.h
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 1.0
 * @date 2022-07-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SRC_CORE_I2C_H_
#define SRC_CORE_I2C_H_

#include <msp430.h>
#include <stdint.h>
#include <src/config.h>

#define MAX_RX_BUFFER_SIZE         80
#define MAX_TX_BUFFER_SIZE         50

#define I2C_TIMEOUT                140
typedef enum I2C_ModeEnum{
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    SWITCH_TO_RX_MODE,
    SWITCH_TO_TX_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    TIMEOUT_MODE,
    SLAVE_ADDR_RX
} I2C_Mode;

typedef struct{
    uint8_t dev_addr;
    uint8_t reg_addr;
    uint8_t data_count;
    uint8_t data[];
}i2c_data_t;

void i2cInit_MasterB0(void);

void i2cInit_MasterB2(void);

/**
 * Init i2c as Master
 * @return
 */
void i2cInit_MasterB1(void);

/**
 *
 * @param dev_addr
 * @param reg_addr
 * @param reg_data
 * @param count
 * @return
 */
I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count, uint8_t sel_bus);

/**
 *
 * @param dev_addr
 * @param reg_addr
 * @param count
 * @return
 */
I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count, uint8_t *data, uint8_t sel_bus);

/**
 * A function that created specifically for the UBLOX message format.
 * First write a message with variable payload and the reads back.
 */
I2C_Mode I2C_Master_WriteMessUblox(uint8_t dev_addr, uint8_t reg_addr, uint8_t *tx_data, uint8_t *rx_data, uint8_t tx_count, uint8_t rx_count, uint8_t ign_flag, uint8_t sel_bus);

/**
 * Reset B2 i2c bus
 */
void i2cResetB1(void);

#endif /* SRC_CORE_I2C_H_ */
