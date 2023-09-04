/**
 * @file uart.c
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 0.1
 * @date 2022-06-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "timer.h"
#include "ring_buffer.h"

static RB_t RB_uart_rx;
static RB_t RB_uart_tx;

static RB_t DB_uart_rx;
static RB_t DB_uart_tx;

static RB_t *RB_main_rx;
static RB_t *RB_main_tx;

volatile uint8_t transEnd_debug = 0;
volatile uint8_t transEnd_iris = 0;
volatile uint8_t transEnd_nyx = 0;
volatile uint8_t *transEnd_flag_ptr;

volatile unsigned int *UART_IE_VEC;
volatile unsigned int *UART_IFG_VEC;

static volatile uint8_t tmp;
static volatile uint8_t prv_tmp = 0x00;

uint8_t start_char = STX;
uint8_t stop_char = ETX;
uint8_t buffer_size = UART_RB_SIZE;


// #################### UART FUNCTIONS ####################
void uart_init(UART_select uart) {
  RB_init(&RB_uart_rx, UART_RB_SIZE);  // Init RingBuffer object
  RB_init(&RB_uart_tx, UART_RB_SIZE);

  RB_init(&DB_uart_rx, UART_DB_SIZE);  // Init RingBuffer object
  RB_init(&DB_uart_tx, UART_DB_SIZE);

  switch (uart) {
    case UART_DEBUG:
      P5SEL0 |= (BIT4 | BIT5);
      P5SEL1 &= ~(BIT4 | BIT5);
      UCA2CTLW0 = UCSWRST;         // Put eUSCI in reset
      UCA2CTLW0 |= UCSSEL__SMCLK;  // CLK = SMCLK
      // Baud Rate calculation
      // 115200@16MHz
      UCA2BRW = 8;
      UCA2MCTLW |= 0xF7A1;  // Fractional portion = 0.085 (UserManual table 30-4)
      UCA2CTLW0 &= ~UCSWRST;  // Initialize eUSCI
      UCA2IE |= UCAx_RX_EN;
      start_char = STX;
      break;
    case UART_IRIS:
      P2SEL1 |= (BIT5 | BIT6);
      P2SEL0 &= ~(BIT5 | BIT6);
      UCA1CTLW0 = UCSWRST;         // Put eUSCI in reset
      UCA1CTLW0 |= UCSSEL__SMCLK;  // CLK = SMCLK
      // Baud Rate calculation
      // 115200@16MHz
      UCA1BRW = 8;
      UCA1MCTLW |=
              0xF7A1;  // Fractional portion = 0.085 (UserManual table 30-4)
      UCA1CTLW0 &= ~UCSWRST;  // Initialize eUSCI
      UCA1IE |= UCAx_RX_EN;
      start_char = STX;
      break;
    case UART_NYX:
      P2SEL1 |= (BIT0 | BIT1);
      P2SEL0 &= ~(BIT0 | BIT1);
      UCA0CTLW0 = UCSWRST;         // Put eUSCI in reset
      UCA0CTLW0 |= UCSSEL__SMCLK;  // CLK = SMCLK
      // Baud Rate calculation
      // 115200@16MHz
      UCA0BRW = 8;
      UCA0MCTLW |=
              0xF7A1;  // Fractional portion = 0.16666 (UserManual table 30-4)
      UCA0CTLW0 &= ~UCSWRST;  // Initialize eUSCI
      UCA0IE |= UCAx_RX_EN;
      start_char = STX;
      break;
    case UART_UBLOX:
      break;
    default:
      break;
  }
}

void uart_select(UART_select uart) {
  switch (uart) {
    case UART_DEBUG:
      RB_main_rx = &DB_uart_rx;
      RB_main_tx = &DB_uart_tx;
      UCA2IE |= UCAx_RX_EN;  // Enable  USCI_A0 RX and TX interrupts
      UART_IE_VEC = &UCA2IE;
      UART_IFG_VEC = &UCA2IFG;
      transEnd_flag_ptr = &transEnd_debug;
      start_char = STX;
      break;
    case UART_IRIS:
      RB_main_rx = &RB_uart_rx;
      RB_main_tx = &RB_uart_tx;
      UCA1IE |= UCAx_RX_EN;  // Enable  USCI_A0 RX and TX interrupts
      UART_IE_VEC = &UCA1IE;
      UART_IFG_VEC = &UCA1IFG;
      transEnd_flag_ptr = &transEnd_iris;
      start_char = STX;
      break;
    case UART_NYX:
      RB_main_rx = &DB_uart_rx;
      RB_main_tx = &DB_uart_tx;
      UCA0IE |= UCAx_RX_EN;  // Enable  USCI_A0 RX and TX interrupts
      UART_IE_VEC = &UCA0IE;
      UART_IFG_VEC = &UCA0IFG;
      transEnd_flag_ptr = &transEnd_nyx;
      start_char = STX;
      break;
  }
}

void uart_deinit(UART_select uart){
    switch (uart) {
      case UART_DEBUG:
          UCA2IE &= ~UCAx_RX_EN;
          break;
      case UART_IRIS:
          UCA1IE &= ~UCAx_RX_EN;
          break;
      case UART_NYX:
          UCA0IE = 0;
          break;
    }
}

void uart_enable(UART_select uart){
    switch (uart) {
      case UART_DEBUG:
          UCA2IE |= UCAx_RX_EN;
        break;
      case UART_IRIS:
          UCA1IE |= UCAx_RX_EN;
        break;
      case UART_NYX:
          UCA0IE |= UCAx_RX_EN;
        break;
    }
}

static inline uint8_t uart_RXdata_ready(void) { return (*UART_IFG_VEC & UCRXIFG); }

static inline uint8_t uart_TXdata_ready(void) { return (*UART_IFG_VEC & UCTXIFG); }

uint8_t uart_RB_isFull(void) { return RB_isFull(RB_main_rx); }

uint8_t uart_RB_isEmpty(void) { return RB_isEmpty(RB_main_rx); }

uint16_t uart_RX_getSize(void) {return RB_size(RB_main_rx);}

void uart_clear_tx_RB(void) {
  RB_clear(RB_main_tx);
}

void uart_clear_rx_RB(void){
    RB_clear(RB_main_rx);
}

uint8_t uart_sizeofRB(void) { return RB_size(RB_main_rx); }

uint8_t uart_read_RB(uint8_t *buffer, UART_select uart_sel) {
  uint16_t timeout = 0;
  uint8_t start_ch = 0;
  uart_select(uart_sel);
  memset(buffer, 0, buffer_size);
  start_ch = RB_pop(RB_main_rx);
  if (start_ch != STX) {         // Checks whether the received message starts with the proper char.
    RB_clear(RB_main_rx);  // If no, the message gets rejected.
    *transEnd_flag_ptr = 0;
    *UART_IE_VEC |= UCAx_RX_EN;
    return 1;
  }
  timeout = millis();
  while (!(RB_isEmpty(RB_main_rx))) {  // Loads the message to the buffer.
    if (ellapsed_millis(timeout) > 200) {
      *transEnd_flag_ptr = 0;
      RB_clear(RB_main_rx);
      return 1;
    }
    *buffer = RB_pop(RB_main_rx);
    buffer++;
  }
  RB_clear(RB_main_rx);
  *transEnd_flag_ptr = 0;
  *UART_IE_VEC |= UCAx_RX_EN;  // Re-enable RX interrupt when rb is empty.
  return 0;
}

uint8_t uart_read_DEBUG(uint8_t *buffer, uint16_t len, UART_select uart_sel) {
  uint16_t i = 0;
  uart_select(uart_sel);
  memset(buffer, 0, len);
  while (i < len) {  // Loads the message to the buffer.
    *buffer = RB_pop(RB_main_rx);
    buffer++;
    i++;
  }
  RB_clear(RB_main_rx);
  *transEnd_flag_ptr = 0;
  *UART_IE_VEC |= UCAx_RX_EN;  // Re-enable RX interrupt when rb is empty.
  return 0;
}

uint8_t uart_wait_ACK(UART_select uart_sel){
    uint8_t start_ch, token = 0;
    uart_select(uart_sel);
    RB_clear(RB_main_rx);
    uint16_t timeout = millis();
    while (uart_RX_getSize() < 2){
      if (ellapsed_millis(timeout) > 100){
          RB_clear(RB_main_rx);
          *UART_IE_VEC |= UCAx_RX_EN;  // Re-enable RX interrupt when rb is empty.
          return 1;
      }
    }
    start_ch = RB_pop(RB_main_rx);
    token = RB_pop(RB_main_rx);
    RB_clear(RB_main_rx);
    if (start_ch != 6) {
        return 1;
    }
    return 0;
}

uint8_t uart_write_RB(uint8_t *buffer, uint16_t msg_len, UART_select uart_sel) {
//  uint8_t prev_val=STX;
  uint16_t i = 0;
  RB_clear(RB_main_rx);
  uart_select(uart_sel);
  while (i<msg_len) {  // Loads the message to the transmitters ring buffer.
    RB_push(RB_main_tx, *buffer);
//    prev_val = *buffer;
    buffer++;
    i++;
  }
  *transEnd_flag_ptr = 0;
  *UART_IE_VEC |= UCAx_TX_EN;
  return 0;
}

uint8_t uart_write_DEBUG(char *buffer, uint8_t uart_sel) {
  uart_select(uart_sel);
  while (*buffer !=
         '\0') {  // Loads the message to the transmitters ring buffer.
    RB_push(RB_main_tx, *buffer);
    buffer++;
  }
  *transEnd_flag_ptr = 0;
  *UART_IE_VEC |= UCAx_TX_EN;
  return 0;
}

uint8_t uart_pop_RX(void) { return RB_pop(RB_main_rx); }

uint8_t uart_popBack_RX(void) { return RB_popBack(RB_main_rx); }

void uart_push_TX(uint8_t byte) { return RB_push(RB_main_tx, byte); }

void uart_pushFront_TX(uint8_t byte) { return RB_pushFront(RB_main_tx, byte); }



#pragma vector = USCI_A2_VECTOR
__interrupt void USCI_A2_ISR(void) {
  // uint8_t tmp_uart_buffer;
  switch (__even_in_range(UCA2IV, 4)) {
    case 0x00:  // Vector 0: No interrupts
      break;

    case 0x02:
      if (!uart_RXdata_ready()) {
#ifdef MSG_PROT
        tmp = UCA2RXBUF;
        if ((tmp == ETX) && (prv_tmp != ESC)) {
          UCA2IE &= ~UCAx_RX_EN;
          transEnd_debug = 1;
        }
        else if ((tmp == ESC) && (prv_tmp != ESC)){
            prv_tmp = tmp;
        }
        else if ((tmp == ESC) && (prv_tmp == ESC)){
            RB_push(RB_main_rx, tmp);
            prv_tmp = 0x00;
        }
        else {
          RB_push(RB_main_rx, tmp);
          prv_tmp = tmp;
        }
#else
        tmp = UCA2RXBUF;
        RB_push(RB_main_rx, tmp);
        if (tmp == CR) {  // If the received char is Enter or
                                          // the msg stop char,
          UCA2IE &= ~UCAx_RX_EN;          // disable the interrupts
          transEnd_debug = 1;           // and set the transmission-end flag.
        }
#endif
        if (RB_isFull(RB_main_rx)) {  // If the RX ring buffer is full do the same.
          UCA2IE &= ~UCAx_RX_EN;
          transEnd_debug = 1;
        }
      }
      break;
    case 0x04:
      if (!uart_TXdata_ready()) {
        UCA2TXBUF = RB_pop(RB_main_tx);
        if (RB_isEmpty(RB_main_tx)) {
          RB_clear(RB_main_tx);
          UCA2IE &= ~UCAx_TX_EN;
        }
      }
      break;
  }
}


#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
  // uint8_t tmp_uart_buffer;
  switch (__even_in_range(UCA1IV, 4)) {
    case 0x00:  // Vector 0: No interrupts
      break;

    case 0x02:
      if (!uart_RXdata_ready()) {
#ifdef MSG_PROT
          tmp = UCA1RXBUF;
          if ((tmp == ETX) && (prv_tmp != ESC)) {
              UCA1IE &= ~UCAx_RX_EN;
              transEnd_iris = 1;
          }
          else if ((tmp == ESC) && (prv_tmp != ESC)){
              prv_tmp = tmp;
          }
          else if ((tmp == ESC) && (prv_tmp == ESC)){
              RB_push(RB_main_rx, tmp);
              prv_tmp = 0x00;
          }
          else{
              RB_push(RB_main_rx, tmp);
              prv_tmp = tmp;
          }
#else
        tmp = UCA1RXBUF;
        RB_push(RB_main_rx, tmp);
        if (tmp == CR) {  // If the received char is Enter or
                                          // the msg stop char,
          UCA1IE &= ~UCAx_RX_EN;          // disable the interrupts
          transEnd_iris = 1;           // and set the transmission-end flag.
        }
#endif
        if (RB_isFull(
                RB_main_rx)) {  // If the RX ring buffer is full do the same.
          UCA1IE &= ~UCAx_RX_EN;
          transEnd_iris = 1;
        }
      }
      break;
    case 0x04:
      if (!uart_TXdata_ready()) {
        UCA1TXBUF = RB_pop(RB_main_tx);
        if (RB_isEmpty(RB_main_tx)) {
          RB_clear(RB_main_tx);
          UCA1IE &= ~UCAx_TX_EN;
        }
      }
      break;
  }
}


#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void) {
  // uint8_t tmp_uart_buffer;
  switch (__even_in_range(UCA0IV, 4)) {
    case 0x00:  // Vector 0: No interrupts
      break;

    case 0x02:
      if (!uart_RXdata_ready()) {
#ifdef MSG_PROT
          tmp = UCA0RXBUF;
          if ((tmp == ETX) && (prv_tmp != ESC)) {
              UCA0IE &= ~UCAx_RX_EN;
              transEnd_nyx = 1;
          }
          else if ((tmp == ESC) && (prv_tmp != ESC)){
              prv_tmp = tmp;
          }
          else if ((tmp == ESC) && (prv_tmp == ESC)){
              RB_push(RB_main_rx, tmp);
              prv_tmp = 0x00;
          }
          else{
              RB_push(RB_main_rx, tmp);
              prv_tmp = tmp;
          }
#else
        tmp = UCA0RXBUF;
        RB_push(RB_main_rx, tmp);
        if (tmp == CR) {  // If the received char is Enter or
                                          // the msg stop char,
          UCA0IE &= ~UCAx_RX_EN;          // disable the interrupts
          transEnd_nyx = 1;           // and set the transmission-end flag.
        }
#endif
        if (RB_isFull(
                RB_main_rx)) {  // If the RX ring buffer is full do the same.
          UCA0IE &= ~UCAx_RX_EN;
          transEnd_nyx = 1;
        }
      }
      break;
    case 0x04:
      if (!uart_TXdata_ready()) {
        UCA0TXBUF = RB_pop(RB_main_tx);
        if (RB_isEmpty(RB_main_tx)) {
          RB_clear(RB_main_tx);
          UCA0IE &= ~UCAx_TX_EN;
        }
      }
      break;
  }
}
