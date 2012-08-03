/* Copyright 2010-2012 JP Norair
  *
  * Licensed under the OpenTag License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  */
/**
  * @file       /otplatform/cc430/mpipe_CC430_uart.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 July 2011
  * @brief      Message Pipe (MPIPE) implementation(s) for CC430
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * @note This driver should be virtually identical to the one for MSP430F5
  * 
  * As far as I know, the CC430 has the ability to use the following peripherals
  * for the MPIPE.  This list may change over time as new models of the CC430
  * are released:
  *     I/F     HW      Impl.   Baudrate Notes
  * 1.  UART    USCI    yes     Keep baudrate <= 115.2 kbps
  * 2.  SPI     USCI    no      Potentially up to 5 Mbps, using SMCLK
  * 3.  I2C     USCI    no      100 or 400 kbps
  * 4.  IrDA    USCI    no      Need more information
  *
  * The UART implementation is the only one presently implemented.
  * Baudrates supported:    9600, 28800, 57600, 115200
  * Byte structure:         8N1
  * Duplex:                 Half
  * Flow control:           None
  * Connection:             RS-232, DTE-DTE (use a null-modem connector)
  *
  * Design Assumptions:
  * - Using SMCLK at 2.49625 MHz Clock (19.97 MHz / 8)
  * - Using UART0
  * - If changing the input frequency, changes need to be made to implementation
  *   of setspeed function.
  * - If changing to another UART, changes to platform_config_CC430.h and to
  *   some macros in this file will be needed
  *
  * Implemented Mpipe Protocol:
  * The Mpipe protocol is a simple wrapper to NDEF.
  * Legend: [ NDEF Header ] [ NDEF Payload ] [ Seq. Number ] [ CRC16 ]
  * Bytes:        6             <= 255             2             2
  *
  * The protocol includes an ACK/NACK feature.  After receiving a message, the
  * Mpipe send an ACK/NACK.  The "YY" byte is 0 for ACK and non-zero for ACK.
  * Presently, 0x7F is used as the YY NACK value.
  * [ Seq ID ] 0xDD 0x00 0x00 0x02 0x00 0xYY  [ CRC16 ]
  ******************************************************************************
  */


#include "OT_config.h"

#if (OT_FEATURE(MPIPE) == ENABLED)

#include "buffers.h"
#include "mpipe.h"
#include "OT_platform.h"

#if ((MPIPE_UARTNUM != 0) && (MPIPE_UARTNUM != 1))
#   error "MPIPE_UARTNUM not defined to an available UART"
#endif


#define MPIPE_USE_ACKS      (PLATFORM_FEATURE_USBCONVERTER != ENABLED)

#define UART_CLOSE()        (MPIPE_UART->CTL1   |= UCSWRST)
#define UART_OPEN()         (MPIPE_UART->CTL1   &= ~UCSWRST)
#define UART_SET_TXIFG()    (MPIPE_UART->IFG    |= UCTXIFG)
#define UART_CLEAR_TXIFG()  (MPIPE_UART->IFG    &= ~UCTXIFG)
#define UART_SET_RXIFG()    (MPIPE_UART->IFG    |= UCRXIFG)
#define UART_CLEAR_RXIFG()  (MPIPE_UART->IFG    &= ~UCRXIFG)



// Setup DMA for RX, and enable it
#define MPIPE_DMA_RXCTL_ON          ( DMA_Mode_Single | \
                                      DMA_DestinationInc_Enable | \
                                      DMA_SourceInc_Disable | \
                                      DMA_DestinationDataSize_Byte | \
                                      DMA_SourceDataSize_Byte | \
                                      DMA_TriggerLevel_RisingEdge | \
                                      0x0014 )

// Setup DMA for RX, but disable / do not enable
#define MPIPE_DMA_RXCTL_OFF         ( DMA_Mode_Single | \
                                      DMA_DestinationInc_Enable | \
                                      DMA_SourceInc_Disable | \
                                      DMA_DestinationDataSize_Byte | \
                                      DMA_SourceDataSize_Byte | \
                                      DMA_TriggerLevel_RisingEdge | \
                                      0x0004 )

// Setup DMA for TX, and enable it
#define MPIPE_DMA_TXCTL_ON          ( DMA_Mode_Single | \
                                      DMA_DestinationInc_Disable | \
                                      DMA_SourceInc_Enable | \
                                      DMA_DestinationDataSize_Byte | \
                                      DMA_SourceDataSize_Byte | \
                                      DMA_TriggerLevel_RisingEdge | \
                                      0x0014 )

// Setup DMA for TX, but disable / do not enable
#define MPIPE_DMA_TXCTL_OFF         ( DMA_Mode_Single | \
                                      DMA_DestinationInc_Disable | \
                                      DMA_SourceInc_Enable | \
                                      DMA_DestinationDataSize_Byte | \
                                      DMA_SourceDataSize_Byte | \
                                      DMA_TriggerLevel_RisingEdge | \
                                      0x0004 )

#define MPIPE_DMA_RXCONFIG(DEST, SIZE, ONOFF) \
    do { \
        MPIPE_DMA->SA_L = (ot_u16)&(MPIPE_UART->RXBUF); \
        MPIPE_DMA->DA_L = (ot_u16)DEST; \
        MPIPE_DMA->SZ   = SIZE; \
        MPIPE_DMA->CTL  = MPIPE_DMA_RXCTL_##ONOFF; \
    } while(0)

///@todo Resolve SIZE+2 workaround for DMA interrupt delay
#define MPIPE_DMA_TXCONFIG(SOURCE, SIZE, ONOFF) \
    do { \
        MPIPE_DMA->SA_L = (ot_u16)SOURCE; \
        MPIPE_DMA->DA_L = (ot_u16)&(MPIPE_UART->TXBUF); \
        MPIPE_DMA->SZ   = SIZE; \
        MPIPE_DMA->CTL  = MPIPE_DMA_TXCTL_##ONOFF; \
    } while(0)

#define MPIPE_DMAEN(ONOFF)  MPIPE_DMA_##ONOFF
#define MPIPE_DMAIE(ONOFF)  MPIPE_DMAIE_##ONOFF
#define MPIPE_DMA_ON        (MPIPE_DMA->CTL |= 0x0010)
#define MPIPE_DMA_OFF       (MPIPE_DMA->CTL &= ~0x0010)
#define MPIPE_DMAIE_ON      (MPIPE_DMA->CTL |= 0x0004)
#define MPIPE_DMAIE_OFF     (MPIPE_DMA->CTL &= ~0x0004)

#define MPIPE_DMA_TXTRIGGER()   do { \
                                    UART_CLEAR_TXIFG(); \
                                    UART_SET_TXIFG(); \
                                } while(0)
//#define MPIPE_DMA_TXTRIGGER()   (MPIPE_DMA->CTL |= 1)



#define MPIPE_FOOTERBYTES   4





/** Mpipe Module Data (used by all Mpipe implementations)   <BR>
  * ========================================================================<BR>
  */
  
typedef struct {
#   if (MPIPE_USE_ACKS)
        ot_u8           ackbuf[10];
        mpipe_priority  priority;
#   endif
    mpipe_state     state;
    ot_uni16        sequence;
    ot_u8*          pktbuf;
    ot_int          pktlen;

#   if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
        ot_sigv sig_rxdone;
        ot_sigv sig_txdone;
        ot_sigv sig_rxdetect;
#   endif
} mpipe_struct;

mpipe_struct    mpipe;
alp_tmpl        mpipe_alp;






/** Mpipe Subs (Board & Platform dependent)  <BR>
  * ========================================================================
  */

void sub_uart_portsetup() {
    // Get write-access to port mapping regs, update, re-lock
    PM->PWD        = 0x2D52;
    MPIPE_UART_TXMAP = MPIPE_UART_TXSIG;
    MPIPE_UART_RXMAP = MPIPE_UART_RXSIG;
    PM->PWD        = 0;

    MPIPE_UART_PORT->SEL   |= MPIPE_UART_PINS;     // Set pins to alternate function
  //MPIPE_UART_PORT->IE    &= ~MPIPE_UART_PINS;    // disable any interrupts on TX/RX pins
    MPIPE_UART_PORT->IFG   &= ~MPIPE_UART_PINS;    // clear any interrupt flags on TX/RX pins
    MPIPE_UART_PORT->DDIR  &= ~MPIPE_UART_PINS;    // Clear direction (set input)
    MPIPE_UART_PORT->DDIR  |= MPIPE_UART_TXPIN;   // Set TX to output
  //MPIPE_UART_PORT->DS    |= MPIPE_UART_TXPIN;   // Set drive strength to high on TX pin

    MPIPE_UART->IE   = 0;
}






/** Mpipe Low-Level ISRs (Platform-dependent)  <BR>
  * ========================================================================
  */

#if (ISR_EMBED(MPIPE) == ENABLED)

#if (CC_SUPPORT == CL430)
#   pragma vector=DMA_VECTOR
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(DMA_VECTOR)
#elif (CC_SUPPORT == IAR_V5)
#else
#   error "A known compiler has not been defined"
#endif
OT_INTERRUPT void mpipe_dma_isr(void) {
    //MPIPE_DMAEN(OFF); //unnecessary on single transfer mode
#   if (MPIPE_DMANUM == 0)
        if (DMA->IV == 2) mpipe_isr();
#   elif (MPIPE_DMANUM == 1)
        if (DMA->IV == 4) mpipe_isr();
#   elif (MPIPE_DMANUM == 2)
        if (DMA->IV == 6) mpipe_isr();
#   else
#       error "This version of MPIPE requires DMA."
#   endif
    LPM4_EXIT;
}


#if (CC_SUPPORT == CL430)
#   pragma vector=MPIPE_UART_VECTOR
#elif (CC_SUPPORT == GCC)
    OT_IRQPRAGMA(MPIPE_UART_VECTOR)
#elif (CC_SUPPORT == IAR_V5)
#else
#   error "A known compiler has not been defined"
#endif
OT_INTERRUPT void mpipe_uart_isr(void) {
    //MPIPE_UART->IFG = 0;
    if ((mpipe.state == MPIPE_Tx_Wait) || (mpipe.state == MPIPE_TxAck_Wait)) {
        mpipe.state++;
    }
    else {
        mpipe_isr();
    }
    LPM4_EXIT;
}
#endif






/** Mpipe Callback Configurators   <BR>
  * ========================================================================
  */

#if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
void mpipe_setsig_txdone(ot_sigv signal) {
    mpipe.sig_txdone = signal;
}

void mpipe_setsig_rxdone(ot_sigv signal) {
    mpipe.sig_rxdone = signal;
}

void mpipe_setsig_rxdetect(ot_sigv signal) {
    mpipe.sig_rxdetect = signal;
}
#endif






/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */
#ifndef EXTF_mpipe_footerbytes
ot_u8 mpipe_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif


#ifndef EXTF_mpipe_init
ot_int mpipe_init(void* port_id) {
/// 0. "port_id" is unused in this impl, and it may be NULL
/// 1. Set all signal callbacks to NULL, and initialize other variables.
/// 2. Prepare the HW, which in this case is a UART
/// 3. Set default speed, which in this case is 115200 bps

#if (OT_FEATURE(MPIPE_CALLBACKS) == ENABLED)
    mpipe.sig_rxdone    = &otutils_sigv_null;
    mpipe.sig_txdone    = &otutils_sigv_null;
    mpipe.sig_rxdetect  = &otutils_sigv_null;
#endif
#if (MPIPE_USE_ACKS)
    mpipe.ackbuf[0] = 0xDD;     //NDEF message flags
    mpipe.ackbuf[1] = 0;        //Typelen = 0
    mpipe.ackbuf[2] = 0;        //Paylod len = 0
    mpipe.ackbuf[3] = 2;        //ID len = 2
    mpipe.ackbuf[4] = 0;
    mpipe.priority  = MPIPE_Low;
#endif
    mpipe.state     = MPIPE_Idle;
    mpipe_alp.inq   = &dir_in;
    mpipe_alp.outq  = &dir_out;

    //mpipe.sequence.ushort   = 0;          //not actually necessary

    sub_uart_portsetup();
    mpipe_setspeed(MPIPE_115200bps);     //default baud rate

    return 0;
}
#endif



#ifndef EXTF_mpipe_kill
void mpipe_kill() {
/// Figure out what to put in here
}
#endif



#ifndef EXTF_mpipe_wait
void mpipe_wait() {
    while (mpipe.state != MPIPE_Idle) {
        SLEEP_MCU();
    }
}
#endif



#ifndef EXTF_mpipe_setspeed
void mpipe_setspeed(mpipe_speed speed) {
    ot_u8* baud_data;
    /// You will need to change this baud rate matrix if
    /// you change the bit clock from SMCLK @ 2.496 MHz
    /// Order is [ BR0, BR1, MCTL, ... ]
    /// BR1:0 = Floor(BitCLK/BaudRate)
    /// MCTL  = Round(((BitCLK/BaudRate) - Floor(BitCLK/BaudRate)) * 8) << 1
    static const ot_u8 baud_matrix[] = { 0x04, 0x01, 0x00, 0, \
                                         0x56, 0x00, 0x0A, 0, \
                                         0x2B, 0x00, 0x06, 0, \
                                         0x15, 0x00, 0x0A, 0  };

#   if (MCU_FEATURE(MPIPEDMA) == ENABLED)
        MPIPE_DMAEN(OFF);
#   else
#       error "Mpipe requires a DMA in this implementation"
#   endif

    UART_CLOSE();

    // Parity off, LSB-first, 8N1, async uart
    // Use SMCLK and keep UART reset
    MPIPE_UART->CTL0 = 0;
    MPIPE_UART->CTL1 = 0x81;
    MPIPE_UART->IE   = 0;
    MPIPE_UART->IFG  = 0;
    baud_data        = (ot_u8*)&baud_matrix[speed << 2];
    MPIPE_UART->BR0  = *baud_data++;
    MPIPE_UART->BR1  = *baud_data++;
    MPIPE_UART->MCTL = *baud_data;
}
#endif



#ifndef EXTF_mpipe_status
mpipe_state mpipe_status() {
    return mpipe.state;
}
#endif




#ifndef EXTF_mpipe_txndef
void mpipe_txndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority) {
/// Data TX will only occur if this function is called when the MPipe state is
/// idle.  The exception is when the function is called with ACK priority, in
/// which case the state doesn't need to be Idle.  Lastly, if you specify the 
/// blocking parameter, the function will not return until the packet is 
/// completely transmitted.
    ot_uni16    crcval;
    ot_int      data_length;

#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack)) {
        mpipe.priority  = data_priority;
        goto mpipe_txndef_SETUP;
    }
#endif
    if (mpipe.state == MPIPE_Idle) {
        mpipe.state     = MPIPE_Tx_Wait;
        mpipe_txndef_SETUP:
        mpipe.pktbuf    = data;
        data_length     = data[2] + 6;
        //mpipe.pktlen    = data[2] + 6;
        MPIPE_DMAEN(OFF);

#       if (MPIPE_DMANUM == 0)
        DMA->CTL0  |= MPIPE_UART_TXTRIG;
#       elif (MPIPE_DMANUM == 1)
        DMA->CTL0  |= (MPIPE_UART_TXTRIG << 8);
#       elif (MPIPE_DMANUM == 2)
        DMA->CTL1   = MPIPE_UART_TXTRIG;
#       endif

        DMA->CTL4 = (   DMA_Options_RMWDisable | \
                        DMA_Options_RoundRobinDisable | \
                        DMA_Options_ENMIEnable  );

        // add sequence id & crc to end of the datastream
        data[data_length++] = mpipe.sequence.ubyte[UPPER];
        data[data_length++] = mpipe.sequence.ubyte[LOWER];
        crcval.ushort       = platform_crc_block(data, data_length);
        data[data_length++] = crcval.ubyte[UPPER];
        data[data_length++] = crcval.ubyte[LOWER];

        MPIPE_DMA_TXCONFIG(data, data_length, ON);
        UART_OPEN();
        MPIPE_DMA_TXTRIGGER();

        if (blocking) {
           mpipe_wait();
        }
    }
}
#endif




#ifndef EXTF_mpipe_rxndef
void mpipe_rxndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority) {
#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack) {
        mpipe.priority  = data_priority;
        goto mpipe_rxndef_SETUP;
    }
#endif
    if (blocking) {
        mpipe_wait();
    }
    if (mpipe.state == MPIPE_Idle) {
        //mpipe.state     = MPIPE_Idle;
        mpipe_rxndef_SETUP:
        mpipe.pktbuf    = data;
        mpipe.pktlen    = 6;
        MPIPE_DMAEN(OFF);

#       if (MPIPE_DMANUM == 0)
        DMA->CTL0  |= MPIPE_UART_RXTRIG;
#       elif (MPIPE_DMANUM == 1)
        DMA->CTL0  |= (MPIPE_UART_RXTRIG << 8);
#       elif (MPIPE_DMANUM == 2)
        DMA->CTL1   = MPIPE_UART_RXTRIG;
#       endif

        DMA->CTL4 = (   DMA_Options_RMWDisable | \
                        DMA_Options_RoundRobinDisable | \
                        DMA_Options_ENMIEnable  );

        MPIPE_DMA_RXCONFIG(data, 10, ON);
        UART_OPEN();
        UART_CLEAR_RXIFG();
    }
}
#endif




#ifndef EXTF_mpipe_isr
void mpipe_isr() {
/// MPipe is state-based.  Depending on the MPipe implementation and the HW
/// implementation of the DMA+UART, state transitions may happen differently.
/// <LI> In typical RX, there is a header detection event that sets-up a second
///      RX process for downloading the rest of the packet.  When the DMA is
///      done, the process completes.  </LI>
/// <LI> For TX, there is a wait-state needed while the HW UART finishes
///      sending the DMA buffered data (two bytes). </LI>
/// <LI> If MPipe does not have HW acks, then software can be used to manage
///      Acks.  In this case, a complete TX process also requires RX'ing an
///      Ack, and a complete RX process requires TX'ing an Ack. </LI>

    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!
#           if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_rxdetect))
                mpipe.sig_rxdetect(0);  
#           elif defined(EXTF_mpipe_sig_rxdetect)
                mpipe_sig_rxdetect(0);
#           endif

        case MPIPE_RxHeader: 
            mpipe.state     = MPIPE_RxPayload;
            mpipe.pktlen   += mpipe.pktbuf[2] + MPIPE_FOOTERBYTES;
            //@note DMA doesn't seem to need intermediate disabling here
            MPIPE_DMA_RXCONFIG(mpipe.pktbuf+6, mpipe.pktlen-6, ON);
            return;

        case MPIPE_RxPayload:
#           if (MPIPE_USE_ACKS)
            // ACKs must be used when Broadcast mode is off
            if (mpipe.priority != MPIPE_Broadcast) {
                // 1. On ACKs, txndef() requires caller to choose state 
                // 2. Copy RX'ed sequence number into local sequence number
                // 3. Copy NACK/ACK status to 6th byte in NDEF header
                {
                    ot_u8* scratch;
                    mpipe.state                 = MPIPE_TxAck_Wait;
                    scratch                     = &mpipe.pktbuf[mpipe.pktlen-MPIPE_FOOTERBYTES];
                    mpipe.sequence.ubyte[UPPER] = *scratch++;
                    mpipe.sequence.ubyte[LOWER] = *scratch;
                }
                mpipe.ackbuf[5] = (platform_crc_block(mpipe.pktbuf, mpipe.pktlen)) ? 0x7F : 0;
                mpipe_txndef(mpipe.ackbuf, False, MPIPE_Ack);
                return;
            }
#           endif
            goto mpipe_isr_RXDONE;

        case MPIPE_TxAck_Wait:
            MPIPE_UART->IE = UCTXIE;
            return;

        case MPIPE_TxAck_Done:  // TX'ed an ACK
            MPIPE_UART->IE = 0;
#           if (MPIPE_USE_ACKS)
            if (mpipe.ackbuf[5] != 0) { // TX'ed a NACK
                mpipe_rxndef(mpipe.pktbuf, False, mpipe.priority);
                mpipe.state = MPIPE_RxHeader;
                return;
            }
#           endif
            goto mpipe_isr_RXDONE;

        case MPIPE_Tx_Wait:
            MPIPE_UART->IE = UCTXIE;
            return;

        case MPIPE_Tx_Done:
            MPIPE_UART->IE = 0;
#           if (MPIPE_USE_ACKS)
            if (mpipe.priority != MPIPE_Broadcast) {
                mpipe_rxndef(mpipe.ackbuf, False, MPIPE_Ack);
                mpipe.state = MPIPE_RxAck;
                return;
            }
            goto mpipe_isr_TXDONE;  // Broadcast, so no ACK
#           endif
            
        case MPIPE_RxAck:
#           if (MPIPE_USE_ACKS)
            if (platform_crc_block(mpipe.ackbuf, 10) != 0) { //RX'ed NACK
                mpipe_txndef(mpipe.pktbuf, False, mpipe.priority);
                break;
            }
#           endif
            goto mpipe_isr_TXDONE;  //RX'ed ACK
            
       default: mpipe_kill();
                return;
    }
    
    // This is a stack-less RX-Done subroutine
    mpipe_isr_RXDONE:
    UART_CLOSE();
    mpipe.state = MPIPE_Idle;
#   if (MPIPE_USE_ACKS)
        mpipe.priority = MPIPE_Low;
#   endif
#   if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_rxdone))
        mpipe.sig_rxdone(0);
#   elif defined(EXTF_mpipe_sig_rxdone)
        mpipe_sig_rxdone(0);
#   endif
    return;
    
    // This is a stack-less TX-Done subroutine
    mpipe_isr_TXDONE:
    UART_CLOSE();
    mpipe.state = MPIPE_Idle;
    mpipe.sequence.ushort++;    //increment sequence on TX Done
#   if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_txdone))
        mpipe.sig_txdone(0);
#   elif defined(EXTF_mpipe_sig_txdone)
        mpipe_sig_txdone(0);
#   endif
}
#endif




#endif

