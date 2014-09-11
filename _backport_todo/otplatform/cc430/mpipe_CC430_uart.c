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
  * @version    R102
  * @date       1 Nov 2012
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
  * <LI> Using SMCLK at 2.49625 MHz Clock (19.97 MHz / 8) </LI>
  * <LI> If changing the input frequency, changes need to be made to 
  *         implementation of setspeed function. </LI>
  * <LI> If changing to another UART, changes to platform_config_CC430.h and to
  *         some macros in this file will be needed </LI>
  *
  * Implemented Mpipe Protocol: The Mpipe protocol is a simple wrapper to NDEF.
  * <PRE>
  * Legend: [ NDEF Header ] [ NDEF Payload ] [ Seq. Number ] [ CRC16 ]
  * Bytes:        6             <= 255             2             2 
  * </PRE>
  *
  * The protocol includes an ACK/NACK feature, although this is only of any
  * importance if you have a lossy link between client and server.  If you are
  * using a USB->UART converter, USB has a reliable MAC implementation that 
  * eliminates the need for MPipe ACKing.  
  * 
  * Anyway, after receiving a message, the Mpipe send an ACK/NACK.  The "YY" 
  * byte is 0 for ACK and non-zero for ACK.  Presently, 0x7F is used as the YY 
  * NACK value.
  * <PRE>
  * [ Seq ID ] 0xDD 0x00 0x00 0x02 0x00 0xYY  [ CRC16 ]
  * </PRE>
  ******************************************************************************
  */

#include <otplatform.h>

#if (defined(__CC430__) && OT_FEATURE(MPIPE) && defined(MPIPE_UART))

#include <otlib/buffers.h>
#include <otsys/mpipe.h>


#if (MPIPE_UART_ID != 0xA0)
#   error "MPIPE_UART_ID not defined to A0, the only UART available on CC430"
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





/** Mpipe Driver Data  <BR>
  * ========================================================================<BR>
  */
  
typedef struct {
    ot_uni16        seq;
#if (MPIPE_USE_ACKS)
    mpipe_priority  priority;
#endif
} tty_struct;


tty_struct tty;





/** Mpipe Subs (Board & Platform dependent)  <BR>
  * ========================================================================
  */
void sub_txack_header() {
	static const ot_u8 ndef_ack_header[] = {0xDD, 0, 0, 2, 0, 0 };
	q_empty(mpipe.alp.outq);
	q_writestring(mpipe.alp.outq, (ot_u8*)ndef_ack_header, 6);
}


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

OT_INLINE void platform_isr_dma(void) {
#   if (MPIPE_DMANUM == 0)
        if (DMA->IV == 2) mpipedrv_isr();
#   elif (MPIPE_DMANUM == 1)
        if (DMA->IV == 4) mpipedrv_isr();
#   elif (MPIPE_DMANUM == 2)
        if (DMA->IV == 6) mpipedrv_isr();
#   else
#       error "This version of MPIPE requires DMA."
#   endif
}

/*
void platform_isr_uscia0(void) {
    //MPIPE_UART->IFG = 0;
    if ((mpipe.state == MPIPE_Tx_Wait) || (mpipe.state == MPIPE_TxAck_Wait)) {
        mpipe.state++;
    }
    else {
        mpipedrv_isr();
    }
}
*/








/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */
#ifndef EXTF_mpipedrv_footerbytes
ot_u8 mpipedrv_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif


#ifndef EXTF_mpipedrv_init
ot_int mpipedrv_init(void* port_id) {
/// 1. "port_id" is unused in this impl, and it may be NULL
/// 2. Prepare the HW, which in this case is a UART
/// 3. Set default speed, which in this case is 115200 bps

#if (MCU_FEATURE(MEMCPYDMA) != ENABLED)
    BOARD_DMA_COMMON_INIT();
#endif

#if (MPIPE_USE_ACKS)
    tty.priority    = MPIPE_Low;
#endif
    tty.seq.ushort  = 0;          //not actually necessary
    mpipe.state     = MPIPE_Idle;

    alp_init(&mpipe.alp, &otmpin, &otmpout);
    mpipe.alp.inq->back    -= 10;
    mpipe.alp.outq->back   -= 10;

    sub_uart_portsetup();
    mpipedrv_setspeed(MPIPE_115200bps);     //default baud rate

    return 255;
}
#endif


#ifndef EXTF_mpipedrv_standby
void mpipedrv_standby() {
}
#endif


#ifndef EXTF_mpipedrv_detach
void mpipedrv_detach(void* port_id) {
    mpipe.state = MPIPE_Null;
}
#endif



#ifndef EXTF_mpipedrv_block
void mpipedrv_block() {
    mpipe.state = MPIPE_Null;
}
#endif


#ifndef EXTF_mpipedrv_unblock
void mpipedrv_unblock() {
    mpipe.state = MPIPE_Idle;
}
#endif



#ifndef EXTF_mpipedrv_kill
void mpipedrv_kill() {
    MPIPE_DMAEN(OFF);
	UART_CLOSE();
    mpipe.state = MPIPE_Idle;
	q_empty(mpipe.alp.outq);
	mpipe.alp.outq->back -= 10;
}
#endif



#ifndef EXTF_mpipedrv_wait
void mpipedrv_wait() {
    while (mpipe.state != MPIPE_Idle);
}
#endif



#ifndef EXTF_mpipedrv_setspeed
void mpipedrv_setspeed(mpipe_speed speed) {
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
    //MPIPE_UART->IE   = 0;
    //MPIPE_UART->IFG  = 0;
    baud_data        = (ot_u8*)&baud_matrix[speed << 2];
    MPIPE_UART->BR0  = *baud_data++;
    MPIPE_UART->BR1  = *baud_data++;
    MPIPE_UART->MCTL = *baud_data;
}
#endif




#ifndef EXTF_mpipedrv_txndef
void mpipedrv_txndef(ot_bool blocking, mpipe_priority data_priority) {
/// Data TX will only occur if this function is called when the MPipe state is
/// idle.  The exception is when the function is called with ACK priority, in
/// which case the state doesn't need to be Idle.  Lastly, if you specify the 
/// blocking parameter, the function will not return until the packet is 
/// completely transmitted.
    ot_u16 scratch;
    ot_u8* data;

#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack)) {
        tty.priority  = data_priority;
        goto mpipedrv_txndef_SETUP;
    }
#endif
    if (mpipe.state == MPIPE_Idle) {
        mpipe.state     = MPIPE_Tx_Done; //MPIPE_Tx_Wait;
        mpipedrv_txndef_SETUP:
        MPIPE_DMAEN(OFF);
        UART_CLOSE();

#       if (MPIPE_DMANUM == 0)
        DMA->CTL0  |= MPIPE_UART_TXTRIG;
#       elif (MPIPE_DMANUM == 1)
        DMA->CTL0  |= (MPIPE_UART_TXTRIG << 8);
#       elif (MPIPE_DMANUM == 2)
        DMA->CTL1   = MPIPE_UART_TXTRIG;
#       endif

        q_writeshort(mpipe.alp.outq, tty.seq.ushort);                // Sequence Number

        scratch = mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor;    //data length
        scratch = crc16drv_block(mpipe.alp.outq->getcursor, scratch);   //CRC value
        q_writeshort(mpipe.alp.outq, scratch);                              //Put CRC
        
        scratch                     = mpipe.alp.outq->putcursor \
                                    - mpipe.alp.outq->getcursor;            //data length w/ CRC
        data                        = mpipe.alp.outq->getcursor;            //data start
        mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;            //move queue past packet

        // DMA setup
        MPIPE_DMA_TXCONFIG(data, scratch, ON);
        UART_OPEN();
        MPIPE_DMA_TXTRIGGER();

        if (blocking) {
           mpipedrv_wait();
        }
    }
}
#endif




#ifndef EXTF_mpipedrv_rxndef
void mpipedrv_rxndef(ot_bool blocking, mpipe_priority data_priority) {
#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack) {
        tty.priority  = data_priority;
        goto mpipedrv_rxndef_SETUP;
    }
#endif
    if (blocking) {
        mpipedrv_wait();
    }
    if (mpipe.state == MPIPE_Idle) {
        mpipedrv_rxndef_SETUP:
        MPIPE_DMAEN(OFF);
        UART_CLOSE();

#       if (MPIPE_DMANUM == 0)
        DMA->CTL0  |= MPIPE_UART_RXTRIG;
#       elif (MPIPE_DMANUM == 1)
        DMA->CTL0  |= (MPIPE_UART_RXTRIG << 8);
#       elif (MPIPE_DMANUM == 2)
        DMA->CTL1   = MPIPE_UART_RXTRIG;
#       endif

        q_empty(mpipe.alp.inq);
        //mpipe.alp.inq->back -=10;
        MPIPE_DMA_RXCONFIG(mpipe.alp.inq->front, 6, ON);
        UART_OPEN();
        UART_CLEAR_RXIFG();
    }
}
#endif




#ifndef EXTF_mpipedrv_isr
void mpipedrv_isr() {
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
#if (BOARD_FEATURE_USBCONVERTER != ENABLED)
    ot_u16 crc_result;
#endif

    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!
        
        case MPIPE_RxHeader: {
            ot_u8* payload_front;
            ot_int payload_len;
            mpipe.state             = MPIPE_RxPayload;
            payload_len             = mpipe.alp.inq->front[2];
            payload_front           = mpipe.alp.inq->front + 6;
            mpipe.alp.inq->back     = payload_front + payload_len;
            payload_len            += MPIPE_FOOTERBYTES;
         //#mpipe.alp.inq->length   = payload_len + 6;
         	mpipe_alp.inq->putcursor= payload_len + 6;
            MPIPE_DMA_RXCONFIG(payload_front, payload_len, ON);
            mpipeevt_rxdetect(30);      ///@todo make dynamic: this is relevant for 115200bps
        }   return;

        case MPIPE_RxPayload: {
            ot_u8* footer;
            footer                  = mpipe.alp.inq->back;
            tty.seq.ubyte[UPPER]    = *footer++;
            tty.seq.ubyte[LOWER]    = *footer;
            
            // CRC is Good (==0) or bad (!=0) Discard the packet if bad
#           if (BOARD_FEATURE_USBCONVERTER != ENABLED)
            crc_result = crc16drv_block(mpipe.alp.inq->front, q_length(mpipe.alp.inq));
#           endif
            
#           if (MPIPE_USE_ACKS)
            // ACKs must be used when Broadcast mode is off
            // 1. On ACKs, txndef() requires caller to choose state 
            // 2. Copy RX'ed seq number into local seq number
            // 3. Copy NACK/ACK status to 6th byte in NDEF header
            if (tty.priority != MPIPE_Broadcast) {
                mpipe.state = MPIPE_TxAck_Done; //MPIPE_TxAck_Wait;
                sub_txack_header(crc_result);
                mpipedrv_txndef(False, MPIPE_Ack);
                return;
            }
#           endif
        } goto mpipedrv_isr_RXDONE;

        //case MPIPE_TxAck_Wait:
            //MPIPE_UART->IE = UCTXIE;
            //return;

        case MPIPE_TxAck_Done:  // TX'ed an ACK
#           if (MPIPE_USE_ACKS)
            if (mpipe.alp.outq->front[5] != 0) { // TX'ed a NACK
                mpipedrv_rxndef(False, tty.priority);
                mpipe.state = MPIPE_RxHeader;
                return;
            }
            tty.priority = MPIPE_Low;
            goto mpipedrv_isr_RXDONE;
#           endif

        //case MPIPE_Tx_Wait:
            //MPIPE_UART->IE = UCTXIE;
            //return;

        case MPIPE_Tx_Done:
            //MPIPE_UART->IE = 0;
#           if (MPIPE_USE_ACKS)
            if (tty.priority != MPIPE_Broadcast) {
                mpipedrv_rxndef(False, MPIPE_Ack);
                mpipe.state = MPIPE_RxAck;
                return;
            }
#           endif
            goto mpipedrv_isr_TXDONE;  // Broadcast, so no ACK
            
        case MPIPE_RxAck:
#           if (MPIPE_USE_ACKS)
            if (crc16drv_block(mpipe.alp.inq->front, 10) != 0) { //RX'ed NACK
                mpipedrv_txndef(False, tty.priority);
                return;
            }
            goto mpipedrv_isr_TXDONE;  //RX'ed ACK
#           endif
            
       default: mpipedrv_kill();
                mpipe_open();
                return;
    }
    
    // This is a stack-less RX-Done subroutine
    // Close UART, Close MPipe, Tell kernel to start processing
    mpipedrv_isr_RXDONE:
    mpipe.state = MPIPE_Idle;
#   if (BOARD_FEATURE_USBCONVERTER != ENABLED)
    mpipeevt_rxdone((ot_int)crc_result);
#   else
    mpipeevt_rxdone(0);
#   endif
    return;
    
    // This is a stack-less TX-Done subroutine
    // Increment seq, Close MPipe, Tell kernel to disengage
    mpipedrv_isr_TXDONE:
    tty.seq.ushort++;
    mpipe.state = MPIPE_Idle;
    mpipeevt_txdone(2);
}
#endif




#endif

