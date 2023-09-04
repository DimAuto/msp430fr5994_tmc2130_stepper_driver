/*
 * bus.h
 *
 *  Created on: Feb 25, 2023
 *      Author: kalai
 */

#ifndef SRC_MESSAGE_HANDLER_H_
#define SRC_MESSAGE_HANDLER_H_

#include <stdint.h>
#include <stdlib.h>
#include "core/uart.h"
#include <src/config.h>

#define SBP_S_ID            0x32
#define COMM_PROTOCOL_REV   0X01
#define PROTOCOL_VER        0XFE
#define MAX_RETRIES         0x03


// SBP-S COMMANDS
#define SBP_CMD_HEARTBEAT       0x51
#define SBP_CMD_GNSS_DATA       0x56
#define SBP_CMD_BATTERY_LEVEL   0x70
#define SBP_CMD_MAGN_DATA       0x57
#define SBP_CMD_ACCEL_DATA      0x58
#define SBP_CMD_BSL             0x60
#define SBP_CMD_REPORT_FW       0x65
#define SBP_CMD_EXTERNAL_V      0x72
#define SBP_CMD_SELECTED_PLINE  0x74
#define SBP_CMD_NYX_FLIP_FLAG   0x78
#define SBP_CMD_NYX_FLIP_FLAG   0x82

#define MESSAGE_RETRANSMIT_INTERVAL 300

typedef struct{
    uint8_t protocol_rev[2];
    uint8_t token;
    uint8_t senderID[2];
    uint8_t cmd;
    uint16_t len;
    uint8_t data[20];
    uint8_t checksum[4];
}message_t;

/**
 * Ticks the message handler
 */
void tick_Handler(void);

uint8_t get_connected_flag(void);

void set_connected_flag(uint8_t state);

/**
 * Sends a two byte NACK
 * @param device: the desired uart interface
 */
uint8_t sendNack(UART_select device);

/**
 * Sends a two byte ACK
 * @param device: the desired uart interface
 */
uint8_t sendAck(UART_select device);

/**
 * Transmits a message of the required form.
 * @param msg: pointer to the message_t struct.
 * @param data: pointer to the data to be sent
 * @param data_len: the length of the data to be send.
 * @param device: the desired uart interface.
 */
uint8_t transmitMessage(uint8_t *data, uint8_t data_len, uint8_t cmd, UART_select device);

/**
 * Parses the message into the message struct.
 * @param msg
 * @param buffer
 * @param device
 * @return
 */
uint8_t parseMessage(uint8_t *data, uint8_t len, UART_select device);


void reportFW(uint8_t cmd, UART_select device);

uint16_t get_heartbeat_timeout(void);

uint8_t send_heartbeat(UART_select device);

#endif /* SRC_MESSAGE_HANDLER_H_ */
