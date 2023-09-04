/**
 * @file uart.h
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 0.1
 * @date 2022-06-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef UART_H_
#define UART_H_

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

#define UCAx_TX_EN              0x02
#define UCAx_RX_EN              0x01

// UART defines
#define ESC_CHAR            0x07
#define STX                 0x02
#define ETX                 0x03
#define ACK                 0x06
#define NACK                0x15
#define ESC                 0x1B
#define CR                  0x0D

typedef enum {
    UART_DEBUG,
    UART_IRIS,
    UART_NYX,
    UART_UBLOX
} UART_select;


//UARTEXT UCA0
//UARTIRIS UCA2
//UARTNYX UCA1



/**
 * Initialize the specified_uart
 * @param uart_sel the uart port to initialize
 */
void uart_init(UART_select uart);

uint16_t uart_RX_getSize(void);

void uart_select(UART_select uart);

void uart_enable(UART_select uart);

void uart_deinit(UART_select uart);

/**
 * Returns True if ring buffer is empty.
 * @return
 */
uint8_t uart_RB_isEmpty(void);

/**
 * Returns True if ring buffer is full.
 * @return
 */
uint8_t uart_RB_isFull(void);

/**
 * Returns the size of ring buffer.
 * @return
 */
uint8_t uart_sizeofRB(void);

/**
 * Reads the ring buffer until no byte left inside and returns a string with the
 * value
 * @return
 */
uint8_t uart_read_RB(uint8_t *buffer, UART_select uart_sel);

/**
 * Clears the ring buffer.
 * @return
 */
void uart_clear_tx_RB(void);

void uart_clear_rx_RB(void);

/**
 * Writes a string to the UART using the ring buffer.
 * @param buffer
 */
uint8_t uart_write_RB(uint8_t *buffer, uint16_t msg_len, UART_select uart_sel);

/**
 * Returns a byte from the RX rb front.
 * @return
 */
uint8_t uart_pop_RX(void);

/**
 * Push a byte  TX rb front.
 * @param byte
 * @return
 */
void uart_push_TX(uint8_t byte);

/**
 * Push a byte to TX rb back.
 * @param byte
 * @return
 */
void uart_pushFront_TX(uint8_t byte);

uint8_t uart_write_DEBUG(char *buffer, uint8_t uart_sel);

uint8_t uart_read_DEBUG(uint8_t *buffer, uint16_t len, UART_select uart_sel);

uint8_t uart_wait_ACK(UART_select uart_sel);

#endif /* UART_H_ */
