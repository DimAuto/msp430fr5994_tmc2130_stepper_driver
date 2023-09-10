/*
 * spi.c
 *
 *  Created on: Aug 30, 2023
 *      Author: dkalaitzakis
 */

//******************************************************************************
//   MSP430FR599x Demo - eUSCI_B1, SPI 4-Wire Master multiple byte RX/TX
//
//
//                   MSP430FR5994
//                 -----------------
//            /|\ |                 |
//             |  |                 |
//             ---|RST          P1.4|-> Slave Reset (GPIO)
//                |                 |
//                |             P5.0|-> Data Out (UCB1SIMO)
//                |                 |
//                |             P5.1|<- Data In (UCB1SOMI)
//                |                 |
//                |             P5.2|-> Serial Clock Out (UCB1CLK)
//                |                 |
//                |             P5.3|-> Slave Chip Select (GPIO)
//
//******************************************************************************

#include <msp430.h>
#include <stdint.h>
#include "spi.h"
#include "gpio.h"
#include "timer.h"



#define MAX_BUFFER_SIZE     20

/* Used to track the state of the software state machine*/
static SPI_Mode MasterMode = IDLE_MODE;

/* The Register Address/Command to use*/
static uint8_t TransmitRegAddr = 0;

/* ReceiveBuffer: Buffer used to receive data in the ISR
 * RXByteCtr: Number of bytes left to receive
 * ReceiveIndex: The index of the next byte to be received in ReceiveBuffer
 * TransmitBuffer: Buffer used to transmit data in the ISR
 * TXByteCtr: Number of bytes left to transfer
 * TransmitIndex: The index of the next byte to be transmitted in TransmitBuffer
 * */
static uint8_t ReceiveBuffer[MAX_BUFFER_SIZE] = {0};
static uint8_t RXByteCtr = 0;
static uint8_t ReceiveIndex = 0;
static uint8_t TransmitBuffer[MAX_BUFFER_SIZE] = {0};
static uint8_t TXByteCtr = 0;
static uint8_t TransmitIndex = 0;


//******************************************************************************
// Device Initialization *******************************************************
//******************************************************************************

void initSPI(void)
{
    P5SEL1 &= ~(BIT0 | BIT1 | BIT2);
    P5SEL0 |= BIT0 | BIT1 | BIT2;
    P5DIR  &= ~(BIT0 | BIT1 | BIT2);
    //Clock Polarity: The inactive state is high
    //MSB First, 8-bit, Master, 3-pin mode, Synchronous
    UCB1CTLW0 = UCSWRST;                       // **Put state machine in reset**
    UCB1CTLW0 |= UCCKPL | UCMSB | UCSYNC
                | UCMST | UCSSEL__SMCLK;      // 3-pin, 8-bit SPI Slave
    UCB1BRW = 0x28;
    //UCB1MCTLW = 0;
    UCB1CTLW0 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCB1IE |= UCRXIE;                          // Enable USCI0 RX interrupt
}

void SendUCB1Data(uint8_t val)
{
    while (!(UCB1IFG & UCTXIFG));              // USCI_B1 TX buffer ready?
    UCB1TXBUF = val;
}


void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t copyIndex = 0;
    for (copyIndex = 0; copyIndex < count; copyIndex++)
    {
        dest[copyIndex] = source[copyIndex];
    }
}

SPI_Mode SPI_Master_WriteReg(Gpio_Pin *CS_Pin, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = (reg_addr | (1<<7));

    //Copy register data to TransmitBuffer
    CopyArray(reg_data, TransmitBuffer, count);

    TXByteCtr = count;
    RXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;

    gpio_writePin(CS_Pin, LOW);
    SendUCB1Data(TransmitRegAddr);
    uint16_t timeout = millis();
    while (MasterMode != IDLE_MODE){
        if (ellapsed_millis(timeout) > 2000){
            MasterMode = SPI_ERROR;
        }
    }
    gpio_writePin(CS_Pin, HIGH);
    return MasterMode;
}

SPI_Mode SPI_Master_ReadReg(Gpio_Pin *CS_Pin, uint8_t reg_addr, uint8_t count)
{
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = (reg_addr | (0<<7));
    RXByteCtr = count;
    TXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;

    gpio_writePin(CS_Pin, LOW);
    SendUCB1Data(TransmitRegAddr);

    uint16_t timeout = millis();
    while (MasterMode != IDLE_MODE){
       if (ellapsed_millis(timeout) > 2000){
           MasterMode = SPI_ERROR;
       }
    }
    gpio_writePin(CS_Pin, HIGH);
    return MasterMode;
}




//******************************************************************************
// SPI Interrupt ***************************************************************
//******************************************************************************

#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
    uint8_t ucb1_rx_val = 0;
    switch(__even_in_range(UCB1IV, USCI_SPI_UCTXIFG))
    {
        case USCI_NONE: break;
        case USCI_SPI_UCRXIFG:
            ucb1_rx_val = UCB1RXBUF;
            UCB1IFG &= ~UCRXIFG;
            switch (MasterMode)
            {
                case TX_REG_ADDRESS_MODE:
                    if (RXByteCtr)
                    {
                        MasterMode = RX_DATA_MODE;   // Need to start receiving now
                        //Send Dummy To Start
                        __delay_cycles(2000000);
                        SendUCB1Data(DUMMY);
                    }
                    else
                    {
                        MasterMode = TX_DATA_MODE;        // Continue to transmision with the data in Transmit Buffer
                        //Send First
                        SendUCB1Data(TransmitBuffer[TransmitIndex++]);
                        TXByteCtr--;
                    }
                    break;

                case TX_DATA_MODE:
                    if (TXByteCtr)
                    {
                      SendUCB1Data(TransmitBuffer[TransmitIndex++]);
                      TXByteCtr--;
                    }
                    else
                    {
                      //Done with transmission
                      MasterMode = IDLE_MODE;
                    }
                    break;

                case RX_DATA_MODE:
                    if (RXByteCtr)
                    {
                        ReceiveBuffer[ReceiveIndex++] = ucb1_rx_val;
                        //Transmit a dummy
                        RXByteCtr--;
                    }
                    if (RXByteCtr == 0)
                    {
                        MasterMode = IDLE_MODE;
                    }
                    else
                    {
                        SendUCB1Data(DUMMY);
                    }
                    break;

                default:
                    __no_operation();
                    break;
            }
            __delay_cycles(1000);
            break;
        case USCI_SPI_UCTXIFG:
            break;
        default: break;
    }
}


