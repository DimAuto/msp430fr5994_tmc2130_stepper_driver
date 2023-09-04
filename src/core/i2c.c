/**
 * @file i2c.c
 * @author Dimitris Kalaitzakis (dkalaitzakis@theon.com)
 * @brief
 * @version 1.0
 * @date 2022-06-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "timer.h"
#include <src/config.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"


//static volatile uint8_t ReceiveBuffer[MAX_BUFFER_SIZE] = {0};
static uint8_t ReceiveBuffer[MAX_RX_BUFFER_SIZE] = {0};
volatile uint8_t RXByteCtr = 0;
volatile uint8_t ReceiveIndex = 0;
volatile uint8_t TransmitBuffer[MAX_TX_BUFFER_SIZE] = {0};
volatile uint8_t TXByteCtr = 0;
volatile uint8_t TransmitIndex = 0;
static uint8_t TransmitRegAddr = 0;
static volatile uint8_t StopBit_flag = 0;
static volatile uint8_t StopRead_flag = 0;
static volatile I2C_Mode i2cMasterMode = IDLE_MODE;
static volatile uint8_t ubloxIgnoreFlag = 0;

volatile uint16_t *I2C_CTL_REG;


//void i2cInit_MasterB1(void){
//    P5SEL1 &= ~(BIT0 | BIT1);                 //Select i2c function for pins
//    P5SEL0 |= BIT0 | BIT1;                 //Select i2c function for pins
//    P5DIR  &= ~(BIT0 | BIT1);
//    UCB1CTLW0 |= UCSWRST;                      // Enable SW reset
//    UCB1CTLW0 |= (UCMST + UCMODE_3 + UCSYNC + UCSSEL_2);           // I2C Master, synchronous mode
////    UCB1CTLW1 |= UCCLTO_3;
//    UCB1BRW = 0x0028;
//    UCB1CTLW0 &= ~UCSWRST;  // Clear the software reset.
////    UCB1IE |= UCNACKIE;
//}

//void i2cResetB1(void){
//    uint16_t mem = UCB1IE;                //Save current IE state.
//    P5SEL1 &= ~(BIT0 | BIT1);                 //Reset pins
//    P5SEL0 &= ~(BIT0 | BIT1);                 //Reset pins
//    UCB1CTLW0 |= UCSWRST;                       //RESET i2c
//    delayMS(10);
//    P5SEL1 &= ~(BIT0 | BIT1);                 //Set pins as I2C
//    P5SEL0 |= BIT0 | BIT1;                 //Set pins as I2C
//    P5DIR  &= ~(BIT0 | BIT1);
//    UCB1CTLW0 &= ~UCSWRST;
//    UCB1IE = mem;
//}

void i2cInit_MasterB2(void){
    P7SEL1 &= ~(BIT0 | BIT1);                 //Select i2c function for pins
    P7SEL0 |= BIT0 | BIT1;                 //Select i2c function for pins
    P7DIR  &= ~(BIT0 | BIT1);
    UCB2CTLW0 |= UCSWRST;                      // Enable SW reset
    UCB2CTLW0 |= (UCMST + UCMODE_3 + UCSYNC + UCSSEL_2);           // I2C Master, synchronous mode
    //UCB1CTLW1 |= UCASTP_2;
    UCB2BRW = 0x0028;
    UCB2CTLW0 &= ~UCSWRST;  // Clear the software reset.
    UCB2IE |= UCNACKIE;
}

void i2cInit_MasterB0(void){
    P1SEL0 &= ~(BIT6 | BIT7);                 //Select i2c function for pins
    P1SEL1 |= BIT6 | BIT7;                 //Select i2c function for pins
    P1DIR  &= ~(BIT6 | BIT7);
    UCB0CTLW0 |= UCSWRST;                      // Enable SW reset
    UCB0CTLW0 |= (UCMST + UCMODE_3 + UCSYNC + UCSSEL_2);           // I2C Master, synchronous mode
    //UCB1CTLW1 |= UCASTP_2;
    UCB0BRW = 0x0028;
    UCB0CTLW0 &= ~UCSWRST;  // Clear the software reset.
//    UCB0IE |= UCNACKIE;
}

void i2c_select_bus(uint8_t bus_sel, uint8_t dev_addr){
    if (bus_sel == 0x00){
        UCB1IE = 0;
        UCB0I2CSA = dev_addr;
        UCB0IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
        UCB0IE &= ~UCRXIE;                       // Disable RX interrupt
        UCB0IE |= UCTXIE;
        I2C_CTL_REG = &UCB0CTLW0;
    }
//    else if (bus_sel == 0x01){
//        UCB2IE = 0;
//        UCB0IE = 0;
//        UCB1I2CSA = dev_addr;
//        UCB1IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
//        UCB1IE &= ~UCRXIE;                       // Disable RX interrupt
//        UCB1IE |= UCTXIE;
//        I2C_CTL_REG = &UCB1CTLW0;
//    }
    else if (bus_sel == 0x02){
        UCB1IE = 0;
        UCB2I2CSA = dev_addr;
        UCB2IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
        UCB2IE &= ~UCRXIE;                       // Disable RX interrupt
        UCB2IE |= UCTXIE;
        I2C_CTL_REG = &UCB2CTLW0;
    }
}

I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count, uint8_t *data, uint8_t sel_bus)
{
    /* Initialize state machine */
    StopRead_flag = 0;
    i2cMasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;
    if (count > MAX_RX_BUFFER_SIZE){
        count = 1;
    }
    RXByteCtr = count;
    TXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;
    memset(ReceiveBuffer,0,MAX_RX_BUFFER_SIZE);
    /* Initialize slave address and interrupts */
                            // Enable TX interrupt
    i2c_select_bus(sel_bus, dev_addr);

    *I2C_CTL_REG |= UCTR + UCTXSTT;             // I2C TX, start condition
 //   __bis_SR_register(LPM0_bits + GIE);              // Enter LPM0 w/ interrupts

    uint16_t timeout = millis();
    while (!StopRead_flag){
        if (ellapsed_millis(timeout) > I2C_TIMEOUT){
            i2cMasterMode = TIMEOUT_MODE;
            return i2cMasterMode;
        }
    }
    memcpy(data, ReceiveBuffer, count);

    return i2cMasterMode;
}

I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count, uint8_t sel_bus)
{
    /* Initialize state machine */
    i2cMasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;

    //Copy register data to TransmitBuffer
    memcpy(TransmitBuffer, reg_data, count);

    TXByteCtr = count;
    RXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;
    StopBit_flag = 0;

    /* Initialize slave address and interrupts */
    i2c_select_bus(sel_bus, dev_addr);                        // Enable TX interrupt
    *I2C_CTL_REG |= UCTR + UCTXSTT;               // I2C TX, start condition
//    __bis_SR_register(LPM0_bits + GIE);              // Enter LPM0 w/ interrupts
    uint16_t timeout = millis();
    while(!(StopBit_flag)){
          if (ellapsed_millis(timeout) > I2C_TIMEOUT){                  //if timeout flag is set then break
              i2cMasterMode = TIMEOUT_MODE;
              return i2cMasterMode;
          }
    }
    return i2cMasterMode;
}


I2C_Mode I2C_Master_WriteMessUblox(uint8_t dev_addr, uint8_t reg_addr, uint8_t *tx_data, uint8_t *rx_data, uint8_t tx_count, uint8_t rx_count, uint8_t ign_flag, uint8_t sel_bus)
{
    /* Initialize state machine */
    i2cMasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;

    //Copy register data to TransmitBuffer
    memset(TransmitBuffer,0,MAX_TX_BUFFER_SIZE);
    memcpy(TransmitBuffer, tx_data, tx_count);

    TXByteCtr = tx_count;
    RXByteCtr = rx_count;
    ReceiveIndex = 0;
    TransmitIndex = 0;
    StopBit_flag = 0;
    StopRead_flag = 0;
    ubloxIgnoreFlag = ign_flag;

    memset(ReceiveBuffer,0,MAX_RX_BUFFER_SIZE);
    i2c_select_bus(sel_bus, dev_addr);                        // Enable TX interrupt
    *I2C_CTL_REG |= UCTR + UCTXSTT;               // I2C TX, start condition
//    __bis_SR_register(LPM0_bits + GIE);              // Enter LPM0 w/ interrupts
    uint16_t timeout = millis();
    while (!StopRead_flag){
        if (ellapsed_millis(timeout) > 2000){
            i2cMasterMode = TIMEOUT_MODE;
            ubloxIgnoreFlag = 0;
            return i2cMasterMode;
        }
    }
    ubloxIgnoreFlag = 0;
    memcpy(rx_data, ReceiveBuffer, rx_count);
    return i2cMasterMode;
}



#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  uint8_t rx_val = 0;

  switch(__even_in_range(UCB0IV,0xC))
  {
      case USCI_NONE:break;                             // Vector 0 - no interrupt
      case USCI_I2C_UCALIFG:break;                      // Interrupt Vector: I2C Mode: UCALIFG
      case USCI_I2C_UCNACKIFG:      // Interrupt Vector: I2C Mode: UCNACKIFG  Not akn interrupt
          break;
      case USCI_I2C_UCSTTIFG:       // Interrupt Vector: I2C Mode: UCSTTIFG Start condition interrupt
          break;
      case USCI_I2C_UCSTPIFG:
          break;                     // Interrupt Vector: I2C Mode: UCSTPIFG Stop condition interrupt
      case USCI_I2C_UCRXIFG0:
              rx_val = UCB0RXBUF;
              if (RXByteCtr)
              {
                //if (ReceiveIndex > MAX_RX_BUFFER_SIZE)ReceiveIndex=0;
                ReceiveBuffer[ReceiveIndex++] = rx_val;
                RXByteCtr--;
              }
              if (RXByteCtr == 1)
              {
                UCB0CTLW0 |= UCTXSTP;
              }
              else if (RXByteCtr == 0)
              {
                UCB0IE &= ~UCRXIE;
                i2cMasterMode = IDLE_MODE;
                StopRead_flag = 1;
//                __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
              }
          break;                      // Interrupt Vector: I2C Mode: UCRXIFG
      case USCI_I2C_UCTXIFG0:
        switch (i2cMasterMode)
        {
        case TX_REG_ADDRESS_MODE:
            UCB0TXBUF = TransmitRegAddr;
            if ((RXByteCtr) && (!TXByteCtr))
                i2cMasterMode = SWITCH_TO_RX_MODE;   // Need to start receiving now
            else
                i2cMasterMode = TX_DATA_MODE;        // Continue to transmission with the data in Transmit Buffer
            break;

        case SWITCH_TO_RX_MODE:
            UCB0IE |= UCRXIE;              // Enable RX interrupt
            UCB0IE &= ~UCTXIE;             // Disable TX interrupt
            i2cMasterMode = RX_DATA_MODE;    // State is to receive data
            UCB0CTLW0 &= ~UCTR;            // Switch to receiver
            UCB0CTLW0 |= UCTXSTT;          // Send repeated start
            if (RXByteCtr == 1)
            {
                //Must send stop since this is the N-1 byte
                while((UCB0CTLW0 & UCTXSTT));
                UCB0CTLW0 |= UCTXSTP;      // Send stop condition
            }
            break;

        case TX_DATA_MODE:
            if (TXByteCtr)
            {
                UCB0TXBUF = TransmitBuffer[TransmitIndex++];
                TXByteCtr--;
            }
            else
            {
                if (RXByteCtr){
                    UCB0CTLW0 |= UCTXSTP;
                    UCB0IE |= UCRXIE;              // Enable RX interrupt
                    UCB0IE &= ~UCTXIE;             // Disable TX interrupt
                    UCB0CTLW0 &= ~UCTR;            // Switch to receiver
                    i2cMasterMode = RX_DATA_MODE;    // State state is to receive data
                    UCB0CTLW0 |= UCTXSTT;
                }
                else {
                    //Done with transmission
                    UCB0CTLW0 |= UCTXSTP;     // Send stop condition
                    StopBit_flag = 1;
                    i2cMasterMode = IDLE_MODE;
                    UCB0IE &= ~UCTXIE;
                }
                    // disable TX interrupt
//                __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
            }
            break;

        default:
            __no_operation();
            break;
        }
          break;                      // Interrupt Vector: I2C Mode: UCTXIFG
      default: break;
    }
  }



#pragma vector=USCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
{
  uint8_t rx_val = 0;

  switch(__even_in_range(UCB2IV,0xC))
  {
      case USCI_NONE:break;                             // Vector 0 - no interrupt
      case USCI_I2C_UCALIFG:break;                      // Interrupt Vector: I2C Mode: UCALIFG
      case USCI_I2C_UCNACKIFG:      // Interrupt Vector: I2C Mode: UCNACKIFG  Not akn interrupt
          break;
      case USCI_I2C_UCSTTIFG:       // Interrupt Vector: I2C Mode: UCSTTIFG Start condition interrupt
          break;
      case USCI_I2C_UCSTPIFG:
          break;                     // Interrupt Vector: I2C Mode: UCSTPIFG Stop condition interrupt
      case USCI_I2C_UCRXIFG0:
              rx_val = UCB2RXBUF;
              if (RXByteCtr)
              {
                //if (ReceiveIndex > MAX_RX_BUFFER_SIZE)ReceiveIndex=0;
                ReceiveBuffer[ReceiveIndex++] = rx_val;
                RXByteCtr--;
              }
              if (RXByteCtr == 1)
              {
                UCB2CTLW0 |= UCTXSTP;
              }
              else if (RXByteCtr == 0)
              {
                UCB2IE &= ~UCRXIE;
                i2cMasterMode = IDLE_MODE;
                StopRead_flag = 1;
//       __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
              }
          break;                      // Interrupt Vector: I2C Mode: UCRXIFG
      case USCI_I2C_UCTXIFG0:
        switch (i2cMasterMode)
        {
        case TX_REG_ADDRESS_MODE:
            UCB2TXBUF = TransmitRegAddr;
            if ((RXByteCtr) && (!TXByteCtr))
                i2cMasterMode = SWITCH_TO_RX_MODE;   // Need to start receiving now
            else
                i2cMasterMode = TX_DATA_MODE;        // Continue to transmission with the data in Transmit Buffer
            break;

        case SWITCH_TO_RX_MODE:
            UCB2IE |= UCRXIE;              // Enable RX interrupt
            UCB2IE &= ~UCTXIE;             // Disable TX interrupt
            i2cMasterMode = RX_DATA_MODE;    // State is to receive data
            UCB2CTLW0 &= ~UCTR;            // Switch to receiver
            UCB2CTLW0 |= UCTXSTT;          // Send repeated start
            if (RXByteCtr == 1)
            {
                //Must send stop since this is the N-1 byte
                while((UCB2CTLW0 & UCTXSTT));
                UCB2CTLW0 |= UCTXSTP;      // Send stop condition
            }
            break;

        case TX_DATA_MODE:
            if (TXByteCtr)
            {
                UCB2TXBUF = TransmitBuffer[TransmitIndex++];
                TXByteCtr--;
            }
            else
            {
                if (RXByteCtr){
                    UCB2CTLW0 |= UCTXSTP;
                    UCB2IE |= UCRXIE;              // Enable RX interrupt
                    UCB2IE &= ~UCTXIE;             // Disable TX interrupt
                    UCB2CTLW0 &= ~UCTR;            // Switch to receiver
                    i2cMasterMode = RX_DATA_MODE;    // State state is to receive data
                    UCB2CTLW0 |= UCTXSTT;
                }
                else {
                    //Done with transmission
                    UCB2CTLW0 |= UCTXSTP;     // Send stop condition
                    StopBit_flag = 1;
                    i2cMasterMode = IDLE_MODE;
                    UCB2IE &= ~UCTXIE;
                }
                    // disable TX interrupt
//                __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
            }
            break;

        default:
            __no_operation();
            break;
        }
          break;                      // Interrupt Vector: I2C Mode: UCTXIFG
      default: break;
    }
  }




