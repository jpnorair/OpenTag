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
  * @file       /otplatform/stm32l1xx/mpipe_uart_STM32L.c
  * @author     JP Norair
  * @version    R100
  * @date       14 Jan 2013
  * @brief      Message Pipe (MPIPE) UART implementation for STM32L1xx
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * The STM32L has the ability to use the following peripherals for the MPIPE:
  *     I/F     HW      Impl.   Baudrate Notes
  * 1.  UART    USART   yes     Capable of 1Mbps @ 16 MHz clock
  * 2.  USB     USB     pend    Full Speed USB using CDC-ACM profile
  * 3.  SPI     SPI     no      Potentially up to 16 Mbps
  * 4.  I2C     I2C     no      100, 400, 1000 kbps, maybe more
  *
  * The UART implementation is the one implemented in this file.
  * Baudrates supported:    9600, 28800, 57600, 115200, (higher possible)
  * Byte structure:         8N1
  * Duplex:                 Half
  * Flow control:           Crossover RTS/CTS
  * Connection:             RS-232, DTE-DTE (use a null-modem connector)
  *
  * Design Assumptions:
  * <LI> The peripheral bus clock is of high-enough frequency to cleanly 
  *         generate 115200 baud -- 1.8432 MHz or higher is recommended.  </LI>
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

#if (defined(__STM32L__) && OT_FEATURE(MPIPE) && defined(MPIPE_UART))

#include "buffers.h"
#include "mpipe.h"


/** Setup constants   <BR>
  * ========================================================================<BR>
  */
  
#define MPIPE_FOOTERBYTES 4

#if (MCU_FEATURE(MULTISPEED) == ENABLED)
///@todo system calls
#   define __REQUEST_FULL_SPEED();  // sys_req_fullspeed()
#   define __DISMISS_FULL_SPEED();  // sys_clr_fullspeed()
#else
#   define __REQUEST_FULL_SPEED();
#   define __DISMISS_FULL_SPEED();
#endif


#if (MPIPE_UART_ID == 1)
#   define _UARTCLK         (((ot_u32)platform_ext.cpu_khz*1000)/BOARD_PARAM_APB2CLKDIV)
#   define _UARTCLK_HS      (PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB2CLKDIV)
#   define _UART_IRQ        USART1_IRQn
#   define _DMARX           DMA1_Channel5
#   define _DMATX           DMA1_Channel4
#   define _DMARX_IRQ       DMA1_Channel5_IRQn
#   define _DMATX_IRQ       DMA1_Channel4_IRQn
#   define _DMARX_IFG       (0x2 << (4*(5-1)))
#   define _DMATX_IFG       (0x2 << (4*(4-1)))
#   define __UART_ISR       platform_isr_usart1
#   define __DMARX_ISR      platform_isr_dma1ch5
#   define __DMATX_ISR      platform_isr_dma1ch4
#   define __UART_CLKON()   (RCC->APB2ENR |= RCC_APB2ENR_USART1EN)
#   define __UART_CLKOFF()  (RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN)

#elif (MPIPE_UART_ID == 2)
#   define _UARTCLK         (((ot_u32)platform_ext.cpu_khz*1000)/BOARD_PARAM_APB1CLKDIV)
#   define _UARTCLK_HS      (PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB1CLKDIV)
#   define _UART_IRQ        USART2_IRQn
#   define _DMARX           DMA1_Channel7
#   define _DMATX           DMA1_Channel6
#   define _DMARX_IRQ       DMA1_Channel7_IRQn
#   define _DMATX_IRQ       DMA1_Channel6_IRQn
#   define _DMARX_IFG       (0x2 << (4*(7-1)))
#   define _DMATX_IFG       (0x2 << (4*(6-1)))
#   define __UART_ISR       platform_isr_usart2
#   define __DMARX_ISR      platform_isr_dma1ch7
#   define __DMATX_ISR      platform_isr_dma1ch6
#   define __UART_CLKON()   (RCC->APB1ENR |= RCC_APB1ENR_USART2EN)
#   define __UART_CLKOFF()  (RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN)

#elif (MPIPE_UART_ID == 3)
#   define _UARTCLK         (((ot_u32)platform_ext.cpu_khz*1000)/BOARD_PARAM_APB1CLKDIV)
#   define _UARTCLK_HS      (PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB1CLKDIV)
#   define _UART_IRQ        USART3_IRQn
#   define _DMARX           DMA1_Channel3
#   define _DMATX           DMA1_Channel2
#   define _DMARX_IRQ       DMA1_Channel3_IRQn
#   define _DMATX_IRQ       DMA1_Channel2_IRQn
#   define _DMARX_IFG       (0x2 << (4*(3-1)))
#   define _DMATX_IFG       (0x2 << (4*(2-1)))
#   define __UART_ISR       platform_isr_usart3
#   define __DMARX_ISR      platform_isr_dma1ch3
#   define __DMATX_ISR      platform_isr_dma1ch2
#   define __UART_CLKON()   (RCC->APB1ENR |= RCC_APB1ENR_USART3EN)
#   define __UART_CLKOFF()  (RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN)

#else
#   error "MPIPE_UART_ID is misdefined, must be 1, 2, or 3."

#endif




/** Peripheral Control Macros  <BR>
  * ========================================================================<BR>
  */

// UART basic control
#define __UART_CLOSE()      (MPIPE_UART->CR1 = 0)
#define __UART_TXOPEN()     (MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_TE | USART_CR1_TCIE))
#define __UART_RXOPEN()     (MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_RE))
#define __UART_CLEAR()      (MPIPE_UART->SR  = 0)


// DMA basic control
#define _DMA_RX_ON          (_DMARX->CCR |= DMA_CCR1_EN)
#define _DMA_RX_OFF         (_DMARX->CCR &= ~DMA_CCR1_EN)
#define _DMA_TX_ON          (_DMATX->CCR |= DMA_CCR1_EN)
#define _DMA_TX_OFF         (_DMATX->CCR &= ~DMA_CCR1_EN)
#define __DMA_RX(ONOFF)     _DMA_RX_##ONOFF
#define __DMA_TX(ONOFF)     _DMA_TX_##ONOFF
#define __DMA_RX_CLEAR()    (DMA1->IFCR = _DMARX_IFG)
#define __DMA_TX_CLEAR()    (DMA1->IFCR = _DMATX_IFG)
#define __DMA_ALL_CLEAR()   (DMA1->IFCR = (_DMARX_IFG | _DMATX_IFG))

// TX / RX configuration of DMAs
#define __DMA_RXCONFIG(DEST, SIZE)  do { \
                                    _DMARX->CCR  &= ~DMA_CCR1_EN; \
                                    _DMARX->CMAR  = (ot_u32)DEST; \
                                    _DMARX->CNDTR = (ot_u16)SIZE; \
                                    } while(0)

#define __DMA_TXCONFIG(SRC, SIZE)   do { \
                                    _DMATX->CCR  &= ~DMA_CCR1_EN; \
                                    _DMATX->CMAR  = (uint32_t)SRC; \
                                    _DMATX->CNDTR = (ot_u16)SIZE; \
                                    } while(0)  

// Software triggering of DMAs
#define __DMA_TXTRIGGER()           __DMA_TX(ON)
#define __DMA_RXTRIGGER()           __DMA_RX(ON)


#if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
#   define __CLR_RTS()      (MPIPE_RTS_PORT->BSRRH = MPIPE_RTS_PIN)
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Null)
#else
#   define __CLR_RTS();
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Idle)
#endif





/** Mpipe Driver Data  <BR>
  * ========================================================================<BR>
  */
  
typedef struct {
    ot_uni16 seq;
#if (MPIPE_USE_ACKS)
    mpipe_priority  priority;
#endif
#if (BOARD_FEATURE_MPIPE_FLOWCTL == ENABLED)
    ot_u16  counter;
    ot_bool use_rtscts;
#endif
} tty_struct;


tty_struct tty;


///@note the MANT+FRAC baud rate selection can be achieved by the div16 term
static const ot_u16 br_hssel[4] = { 
        (_UARTCLK_HS / (9600/16) ), 
        (_UARTCLK_HS / (28800/16) ), 
        (_UARTCLK_HS / (57600/16) ), 
        (_UARTCLK_HS / (115200/16) )
    };


  


/** Mpipe ISRs  <BR>
  * ========================================================================<BR>
  * These are DMA ISRs.  They are macros which are defined in this file, above.
  * In STM32, each DMA channel has its own IRQ, and each peripheral has mapping
  * to different channels.
  */
  
  
void sub_txndef(ot_bool blocking, mpipe_priority data_priority);
void sub_rxndef(ot_bool blocking, mpipe_priority data_priority);

#if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
void __CTS_ISR(void) {
/// In RTS/CTS mode, the MPipe connection is disabled whenever the session is
/// over, thereby setting mpipe.state to Null.  The kernel can make power 
/// optimizations if it knows MPipe is disconnected.
    mpipe.state = MPIPE_Idle;
    if (MPIPE_RTS_PORT->ODR & MPIPE_RTS_PIN) {
        sub_txndef(False, 0);
    }
    else {
        sub_rxndef(False, 0);
        __SET_RTS();   
    }
}
#endif



void __DMARX_ISR(void) {
    __DMA_RX_CLEAR();
    mpipedrv_isr();
}

void __UART_ISR(void) {
/// Used only for TC (transmission complete)
    __DMA_TX(OFF);
    __DMA_TX_CLEAR();
    __UART_CLEAR();
    __UART_CLOSE();
    __UART_CLKOFF();
    mpipedrv_isr();
}






/** Mpipe Main Subroutines   <BR>
  * ========================================================================
  */
  
void sub_uart_portsetup() {
/// The UART GPIOs should be configured as necessary during board and platform
/// initialization.  This function must be called during UART init, and its job
/// is to do any peripheral remapping, interrupt setup, and such things.

    /// MPIPE CTS interrupt (optional)
#   if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
    NVIC->IP[(ot_u32)_CTS_IRQ]          = PLATFORM_NVIC_IO_GROUP;
    NVIC->ISER[(ot_u32)(_CTS_IRQ>>5)]   = (1 << ((ot_u32)_CTS_IRQ & 0x1F));
#   endif

    // MPipe RX DMA Interrupt, always used
    NVIC->IP[(ot_u32)_DMARX_IRQ]        = PLATFORM_NVIC_IO_GROUP;
    NVIC->ISER[(ot_u32)(_DMARX_IRQ>>5)] = (1 << ((ot_u32)_DMARX_IRQ & 0x1F));

    // MPipe TX TC USART Interrupt, always used
    NVIC->IP[(ot_u32)_UART_IRQ]         = PLATFORM_NVIC_IO_GROUP;
    NVIC->ISER[(ot_u32)(_UART_IRQ>>5)]  = (1 << ((ot_u32)_UART_IRQ & 0x1F));
    
    /// Set up DMA channels for RX and TX
    _DMARX->CCR = DMA_CCR1_MINC | DMA_CCR1_PL_HI | DMA_CCR1_TCIE;
    _DMATX->CCR = DMA_CCR1_DIR | DMA_CCR1_MINC | DMA_CCR1_PL_VHI | DMA_CCR1_TCIE;
    
//  _DMARX->CNDTR = 0;                    //buffer size, filled on usage
//  _DMATX->CNDTR = 0;                    //buffer size, filled on usage
//  _DMARX->CMAR  = (uint32_t)(NULL);     //data buffer, filled on usage
//  _DMATX->CMAR  = (uint32_t)(NULL);     //data buffer, filled on usage
    _DMARX->CPAR  = (uint32_t)&MPIPE_UART->DR;
    _DMATX->CPAR  = (uint32_t)&MPIPE_UART->DR;
    
    /// UART Setup (RX & TX setup takes place at time of startup)
    __UART_CLKON();
    MPIPE_UART->BRR     = br_hssel[MPIPE_115200bps];
    MPIPE_UART->CR1     = 0;
    MPIPE_UART->CR2     = 0;
    MPIPE_UART->CR3     = USART_CR3_DMAR | USART_CR3_DMAT;
    __UART_CLKOFF();
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
#   if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
    mpipe.state     = MPIPE_Null;
#   else
    mpipe.state     = MPIPE_Idle;
#   endif

    alp_init(&mpipe.alp, &dir_in, &dir_out);
    mpipe.alp.inq->back    -= 10;
    mpipe.alp.outq->back   -= 10;

    sub_uart_portsetup();
    
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

#ifndef EXTF_mpipedrv_clear
void mpipedrv_clear() {
    __CLR_MPIPE();
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
    __DMA_RX(OFF);
    __DMA_TX(OFF);
    __DMA_ALL_CLEAR();
    __UART_CLEAR();
	__UART_CLOSE();
    __UART_CLKOFF();
    __CLR_MPIPE();
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
    __UART_CLKON();
    MPIPE_UART->BRR = br_hssel[speed];
    __UART_CLKOFF();
}
#endif



void sub_txndef(ot_bool blocking, mpipe_priority data_priority) {
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
        __REQUEST_FULL_SPEED();
        
        q_writeshort(mpipe.alp.outq, tty.seq.ushort);                       //Sequence Number
        scratch = mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor;    //data length
        scratch = platform_crc_block(mpipe.alp.outq->getcursor, scratch);   //CRC value
        q_writeshort(mpipe.alp.outq, scratch);                              //Put CRC
        
        scratch                     = mpipe.alp.outq->putcursor \
                                    - mpipe.alp.outq->getcursor;            //data length w/ CRC
        data                        = mpipe.alp.outq->getcursor;            //data start
        mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;            //move queue past packet

        /// DMA setup: Receive null data but use the interrupt to indicate
        /// exactly when the packet is done.
        __DMA_TXCONFIG(data, scratch);
        __UART_CLKON();
        __UART_TXOPEN();
        __DMA_TXTRIGGER();

        if (blocking) {
           mpipedrv_wait();
        }
    }
}

#ifndef EXTF_mpipedrv_txndef
void mpipedrv_txndef(ot_bool blocking, mpipe_priority data_priority) {
/// Data TX will only occur if this function is called when the MPipe state is
/// idle.  The exception is when the function is called with ACK priority, in
/// which case the state doesn't need to be Idle.  Lastly, if you specify the 
/// blocking parameter, the function will not return until the packet is 
/// completely transmitted.
#if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
    ///@todo implement a short timer with a guard time to prevent CTS collision.
    //disable CTS interrupt
    //counter = x;
    //while (CTS != active) {
    //counter--;
    __SET_RTS();
    //}
    //if (counter != 0) { pull down RTS, enable CTS interrupt, goto TOP }
    //else enable CTS interrupt
#else
    sub_txndef(blocking, data_priority);
#endif
}
#endif



void sub_rxndef(ot_bool blocking, mpipe_priority data_priority) {
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
        __REQUEST_FULL_SPEED();
        q_empty(mpipe.alp.inq);
        //mpipe.alp.inq->back -=10;
        __DMA_RXCONFIG(mpipe.alp.inq->front, 6);
        __UART_CLKON();
        __UART_RXOPEN();
        __DMA_RXTRIGGER();
    }
}

#ifndef EXTF_mpipedrv_rxndef
void mpipedrv_rxndef(ot_bool blocking, mpipe_priority data_priority) {
#if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
    ///@todo wait for CTS
#else
    sub_rxndef(blocking, data_priority);
#endif
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
    ot_bool tx_process = False;
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
            __DMA_RXCONFIG(payload_front, payload_len);
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
        } goto mpipedrv_isr_RXSIG;

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
            goto mpipedrv_isr_RXSIG;
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
            goto mpipedrv_isr_TXSIG;
            
        case MPIPE_RxAck:
#           if (MPIPE_USE_ACKS)
            if (platform_crc_block(mpipe.alp.inq->front, 10) != 0) { //RX'ed NACK
                mpipedrv_txndef(False, tty.priority);
                return;
            }
            goto mpipedrv_isr_TXSIG;
#           endif
            
       default: mpipedrv_kill();
                goto mpipedrv_isr_TXSIG;
    }
    
    mpipedrv_isr_TXSIG:
    __DMA_TX(OFF);
    __DMA_TX_CLEAR();
    tty.seq.ushort++;
    mpipeevt_txdone(0);
    goto mpipedrv_isr_END;
    
    mpipedrv_isr_RXSIG:
    __DMA_RX(OFF);
    __DMA_RX_CLEAR();
#   if (BOARD_FEATURE_USBCONVERTER != ENABLED)
        mpipeevt_rxdone((ot_int)crc_result);
#   else
        mpipeevt_rxdone(0);
#   endif
    
    mpipedrv_isr_END:
    __UART_CLEAR();
    __UART_CLOSE();
    __UART_CLKOFF();
    __CLR_RTS();
    __CLR_MPIPE();
    __DISMISS_FULL_SPEED();
}
#endif








#endif

