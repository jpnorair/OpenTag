/* Copyright 2010-2013 JP Norair
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
  * @file       /otplatform/stm32f10x/mpipe_uart_STM32L10x.c
  * @author     JP Norair
  * @version    R100
  * @date       14 Jan 2013
  * @brief      Message Pipe (MPIPE) UART implementation for STM32L1xx
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * The STM32L has the ability to use the following peripherals for the MPIPE:
  *     I/F     HW      Impl.   Baudrate Notes
  * 1.  UART    USCI    yes     Keep baudrate <= 115.2 kbps
  * 2.  USB     USB     pend    Full Speed USB using CDC-ACM profile
  * 3.  SPI     USCI    no      Potentially up to 16 Mbps
  * 4.  I2C     USCI    no      100, 400, 1000 kbps, maybe more
  * 5.  IrDA    USCI    no      Need more information
  *
  * The UART implementation is the one implemented in this file.
  * Baudrates supported:    9600, 28800, 57600, 115200, (potentially higher)
  * Byte structure:         8N1
  * Duplex:                 Half
  * Flow control:           None
  * Connection:             RS-232, DTE-DTE (use a null-modem connector)
  *
  * Design Assumptions:
  * <LI> The system clock is of high-enough frequency to cleanly generate the 
  *         115200 baud datarate -- 1.8432 MHz or higher is recommended.  </LI>
  * <LI> If changing the input frequency, changes may need to be made to 
  *         implementation of setspeed function. </LI>
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


#include "OT_platform.h"

#if (defined(__CC430__) && OT_FEATURE(MPIPE) && defined(MPIPE_UART))

#include "buffers.h"
#include "mpipe.h"


/** Setup constants   <BR>
  * ========================================================================<BR>
  */

#if (MPIPE_UART_ID == 1)
#   define _UARTCLK         (platform_ext.cpu_hz/BOARD_PARAM_APB2CLKDIV)
#   define _UART_IRQ        USART1_IRQn
#   define _DMARX_IRQ       DMA1_Channel5_IRQn
#   define _DMATX_IRQ       DMA1_Channel4_IRQn
#   define _DMARX_IFG       (0x2 << (4*(5-1)))
#   define _DMATX_IFG       (0x2 << (4*(4-1)))
#   define __DMARX_ISR      platform_dma1ch5_isr
#   define __DMATX_ISR      platform_dma1ch4_isr
#   define __UART_CLKON()   do { RCC->APB2ENR |= RCC_APB2Periph_USART1  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB2ENR &= ~RCC_APB2Periph_USART1 } while(0)

#elif (MPIPE_UART_ID == 2)
#   define _UARTCLK         (platform_ext.cpu_hz/BOARD_PARAM_APB1CLKDIV)
#   define _UART_IRQ        USART2_IRQn
#   define _DMARX_IRQ       DMA1_Channel7_IRQn
#   define _DMATX_IRQ       DMA1_Channel6_IRQn
#   define _DMARX_IFG       (0x2 << (4*(7-1)))
#   define _DMATX_IFG       (0x2 << (4*(6-1)))
#   define __DMARX_ISR      platform_dma1ch7_isr
#   define __DMATX_ISR      platform_dma1ch6_isr
#   define __UART_CLKON()   do { RCC->APB1ENR |= RCC_APB1Periph_USART2  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB1ENR &= ~RCC_APB1Periph_USART2 } while(0)

#elif (MPIPE_UART_ID == 3)
#   define _UARTCLK         (platform_ext.cpu_hz/BOARD_PARAM_APB1CLKDIV)
#   define _UART_IRQ        USART3_IRQn
#   define _DMARX_IRQ       DMA1_Channel3_IRQn
#   define _DMATX_IRQ       DMA1_Channel2_IRQn
#   define _DMARX_IFG       (0x2 << (4*(3-1)))
#   define _DMATX_IFG       (0x2 << (4*(2-1)))
#   define __DMARX_ISR      platform_dma1ch3_isr
#   define __DMATX_ISR      platform_dma1ch2_isr
#   define __UART_CLKON()   do { RCC->APB1ENR |= RCC_APB1Periph_USART3  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB1ENR &= ~RCC_APB1Periph_USART3 } while(0)

#else
#   error "MPIPE_UART_ID is misdefined, must be 1, 2, or 3."

#endif




/** Peripheral Control Macros  <BR>
  * ========================================================================<BR>
  */

// UART basic control
#define _UART_CLOSE()       (MPIPE_UART->CR1 &= ~(USART_CR1_UE | USART_CR1_TCIE))
#define _UART_TXOPEN()      (MPIPE_UART->CR1 |= (USART_CR1_UE | USART_Mode_Tx))
#define _UART_RXOPEN()      (MPIPE_UART->CR1 |= (USART_CR1_UE | USART_Mode_Rx))
#define _UART_CLEAR()       (MPIPE_UART->SR &= ~USART_SR_TC)
    

// DMA basic control
#define _DMA_RX_ON          (MPIPE_DMA_RXCHAN->CCR |= DMA_CCR1_EN)
#define _DMA_RX_OFF         (MPIPE_DMA_RXCHAN->CCR &= ~DMA_CCR1_EN)
#define _DMA_TX_ON          (MPIPE_DMA_TXCHAN->CCR |= DMA_CCR1_EN)
#define _DMA_TX_OFF         (MPIPE_DMA_TXCHAN->CCR &= ~DMA_CCR1_EN)
#define _DMA_RX(ONOFF)      _DMA_RX_##ONOFF
#define _DMA_TX(ONOFF)      _DMA_TX_##ONOFF
#define _DMA_RX_CLEAR()     (MPIPE_DMA->IFCR = _DMARX_IFG)
#define _DMA_TX_CLEAR()     (MPIPE_DMA->IFCR = _DMATX_IFG)
#define _DMA_ALL_CLEAR()    (MPIPE_DMA->IFCR = (_DMARX_IFG | _DMATX_IFG))

// TX / RX configuration of DMAs
#define __DMA_RXCONFIG(DEST, SIZE)  do { \
                                    MPIPE_DMA_RXCHAN->CMAR  = (uint32_t)DEST; \
                                    MPIPE_DMA_RXCHAN->CNDTR = (ot_u16)SIZE; \
                                    } while(0)

#define __DMA_TXCONFIG(SRC, SIZE)   do { \
                                    MPIPE_DMA_TXCHAN->CMAR  = (uint32_t)SRC; \
                                    MPIPE_DMA_TXCHAN->CNDTR = (ot_u16)SIZE; \
                                    } while(0)  

// Software triggering of DMAs
#define __DMA_TXTRIGGER()           do { \
                                    _UART_CLEAR(); \
                                    _DMA_TX(ON);   \
                                    MPIPE_UART->CR1 |= USART_CR1_TCIE; \
                                    } while(0)
                        
#define __DMA_RXTRIGGER()           _DMA_RX(ON)





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








void __DMARX_ISR(void) {
    _DMA_RX_CLEAR();
    mpipedrv_isr();
}

void __DMATX_ISR(void) {
    _DMA_TX_CLEAR();
    mpipedrv_isr();
}






/** Mpipe Main Subroutines   <BR>
  * ========================================================================
  */
void sub_uart_portsetup() {
/// The UART GPIOs should be configured as necessary during board and platform
/// initialization.  This function must be called during UART init, and its job
/// is to do any peripheral remapping, interrupt setup, and such things.

    /// Remap MPIPE UART port to its alternate setting (if specified)
#   if (MPIPE_REMAP == ENABLED)
        GPIO_PinRemapConfig(MPIPE_REMAP_TYPE, ENABLE);
#   endif
    
    
    // MPIPE UART Interrupt, not typically used
    //NVIC->IP[(ot_u32)_UART_IRQ]         = PLATFORM_NVIC_IO_GROUP;
    //NVIC->ISER[(ot_u32)(_UART_IRQ>>5)]  = (1 << ((ot_u32)_UART_IRQ & 0x1F));

    // MPipe DMA Interrupt, always used
    NVIC->IP[(ot_u32)_DMARX_IRQ]        = PLATFORM_NVIC_IO_GROUP;
    NVIC->IP[(ot_u32)_DMATX_IRQ]        = PLATFORM_NVIC_IO_GROUP;
    NVIC->ISER[(ot_u32)(_DMARX_IRQ>>5)] = (1 << ((ot_u32)_DMARX_IRQ & 0x1F));
    NVIC->ISER[(ot_u32)(_DMATX_IRQ>>5)] = (1 << ((ot_u32)_DMATX_IRQ & 0x1F));

    /// Set up DMA channels for RX and TX
    MPIPE_DMA_RXCHAN->CCR   =   DMA_DIR_PeripheralSRC       | \
                                DMA_Mode_Normal             | \
                                DMA_PeripheralInc_Disable   | \
                                DMA_MemoryInc_Enable        | \
                                DMA_PeripheralDataSize_Byte | \
                                DMA_MemoryDataSize_Byte     | \
                                DMA_Priority_High           | \
                                DMA_M2M_Disable             | \
                                DMA_IT_TC;
    
    MPIPE_DMA_TXCHAN->CCR   =   DMA_DIR_PeripheralDST       | \
                                DMA_Mode_Normal             | \
                                DMA_PeripheralInc_Disable   | \
                                DMA_MemoryInc_Enable        | \
                                DMA_PeripheralDataSize_Byte | \
                                DMA_MemoryDataSize_Byte     | \
                                DMA_Priority_High           | \
                                DMA_M2M_Disable             | \
                                DMA_IT_TC;
    
//  MPIPE_DMA_RXCHAN->CNDTR = 0;                    //buffer size, filled on usage
//  MPIPE_DMA_TXCHAN->CNDTR = 0;                    //buffer size, filled on usage
//  MPIPE_DMA_RXCHAN->CMAR  = (uint32_t)(NULL);     //data buffer, filled on usage
//  MPIPE_DMA_TXCHAN->CMAR  = (uint32_t)(NULL);     //data buffer, filled on usage
    MPIPE_DMA_RXCHAN->CPAR  = (uint32_t)&MPIPE_UART->DR;
    MPIPE_DMA_TXCHAN->CPAR  = (uint32_t)&MPIPE_UART->DR;
    
    /// UART Setup (RX & TX setup takes place at time of startup)
    _UART_CLKON();
    MPIPE_UART->CR1     = USART_WordLength_8b | USART_Parity_No;  
    MPIPE_UART->CR2     = USART_StopBits_1;
    MPIPE_UART->CR3     = USART_CR3_DMAR | USART_CR3_DMAT;
    MPIPE_UART->GTPR    = 0;
}







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

#if (MPIPE_USE_ACKS)
    tty.priority    = MPIPE_Low;
#endif
    tty.seq.ushort  = 0;          //not actually necessary
    mpipe.state     = MPIPE_Idle;

    alp_init(&mpipe.alp, &dir_in, &dir_out);
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
    _DMA_RX(OFF);
    _DMA_TX(OFF);
    _DMA_ALL_CLEAR();
	_UART_CLOSE();
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
    static const ot_u16 br_hssel[4] = { 
        (_UARTCLK_HS/9600), 
        (_UARTCLK_HS/28800), 
        (_UARTCLK_HS/57600), 
        (_UARTCLK_HS/115200)
    };
    
    MPIPE_UART->BRR = br_hssel[speed];
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
        mpipe.state = MPIPE_Tx_Done; //MPIPE_Tx_Wait;
        
        mpipedrv_txndef_SETUP:
        q_writeshort(mpipe.alp.outq, tty.seq.ushort);                       //Sequence Number
        scratch = mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor;    //data length
        scratch = platform_crc_block(mpipe.alp.outq->getcursor, scratch);   //CRC value
        q_writeshort(mpipe.alp.outq, scratch);                              //Put CRC
        
        scratch                     = mpipe.alp.outq->putcursor \
                                    - mpipe.alp.outq->getcursor;            //data length w/ CRC
        data                        = mpipe.alp.outq->getcursor;            //data start
        mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;            //move queue past packet

        // DMA setup
        _DMA_TX(OFF);
        _DMA_TXCONFIG(data, scratch);
        _UART_TXOPEN();
        _DMA_TXTRIGGER();

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
        q_empty(mpipe.alp.inq);
        //mpipe.alp.inq->back -=10;
        _DMA_RX(OFF); 
        _DMA_RXCONFIG(mpipe.alp.inq->front, 6);
        _UART_RXOPEN();
        _DMA_RXTRIGGER();
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
            mpipe.alp.inq->length   = payload_len + 6;
            _DMA_RXCONFIG(payload_front, payload_len);
            mpipeevt_rxdetect(30);      ///@todo make dynamic: this is relevant for 115200bps
        }   return;

        case MPIPE_RxPayload: {
            ot_u8* footer;
            footer                  = mpipe.alp.inq->back;
            tty.seq.ubyte[UPPER]    = *footer++;
            tty.seq.ubyte[LOWER]    = *footer;
            
            // CRC is Good (==0) or bad (!=0) Discard the packet if bad
#           if (BOARD_FEATURE_USBCONVERTER != ENABLED)
            crc_result = platform_crc_block(mpipe.alp.inq->front, mpipe.alp.inq->length);
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
            if (platform_crc_block(mpipe.alp.inq->front, 10) != 0) { //RX'ed NACK
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

