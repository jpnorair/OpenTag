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

#ifndef BOARD_PARAM_MPIPE_IFS
#   define BOARD_PARAM_MPIPE_IFS 1
#endif
#if (defined(__STM32L__) && OT_FEATURE(MPIPE) && (BOARD_PARAM_MPIPE_IFS == 1) && defined(MPIPE_UART))

#include "buffers.h"
#include "mpipe.h"



/** MPipe Feature Configuration for UART, considering other modes <BR>
  * ========================================================================<BR>
  * There are multiple MPIPE modes.  Some platforms/boards support multiplexing
  * the MPIPE across different peripherals and with different options that are
  * made available at runtime.
  */
#define MPIPE_FOOTERBYTES   4
#define MPIPE_UARTMODES     ( (BOARD_FEATURE_MPIPE_DIRECT == ENABLED) \
                            + (BOARD_FEATURE_MPIPE_BREAK == ENABLED)  \
                            + (BOARD_FEATURE_MPIPE_CS == ENABLED)     \
                            + (BOARD_FEATURE_MPIPE_FLOWCTL == ENABLED))
#define MPIPE_MODES         (MPIPE_UARTMODES)



/** MPIPE Interrupt Configuration  <BR>
  * ========================================================================<BR>
  * NVIC parameters are negotiated or defaulted, considering the preset config
  * in the platform section.
  */
#ifndef BUILD_NVIC_SUBGROUP_MPIPE
#   define BUILD_NVIC_SUBGROUP_MPIPE 0
#endif

#define _SUBGROUP   (BUILD_NVIC_SUBGROUP_MPIPE & (16 - (16/__CM3_NVIC_GROUPS)))
#define _IRQGROUP   ((PLATFORM_NVIC_IO_GROUP + _SUBGROUP) << 4)

#if ((BOARD_FEATURE_MPIPE_CS == ENABLED) || (BOARD_FEATURE_MPIPE_FLOWCTL == ENABLED))
#   define _CTS_IRQ     
#endif
#if (BOARD_FEATURE_MPIPE_BREAK == ENABLED)
#   define _BREAK_IRQ   
#endif




/** Platform Clock Configuration   <BR>
  * ========================================================================<BR>
  * MPIPE typically requires the system clock to be set to a certain speed in
  * order for the baud rate generation to work.  So, platforms that are using
  * multispeed clocking will need some extra logic in the MPIPE driver to 
  * assure that the clock speed is on the right setting during MPIPE usage.
  */
#if (MCU_FEATURE(MULTISPEED) == ENABLED)
///@todo system calls
#   define __REQUEST_FULL_SPEED();  // sys_req_fullspeed()
#   define __DISMISS_FULL_SPEED();  // sys_clr_fullspeed()
#else
#   define __REQUEST_FULL_SPEED();
#   define __DISMISS_FULL_SPEED();
#endif




/** MPIPE Peripheral Mapping  <BR>
  * ========================================================================<BR>
  * Use the correct UART and DMA, depending on Board settings.
  */
#if (MPIPE_UART_ID == 1)
#   define _UARTCLK         (((ot_u32)platform_ext.cpu_khz*1000)/BOARD_PARAM_APB2CLKDIV)
#   define _UARTCLK_HS      (PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB2CLKDIV)
#   define _UART_IRQ        USART1_IRQn
#   define _DMARX           DMA1_Channel5
#   define _DMATX           DMA1_Channel4
#   define _DMARX_IRQ       DMA1_Channel5_IRQn
#   define _DMATX_IRQ       DMA1_Channel4_IRQn
#   define _DMARX_IFG       (0xF << (4*(5-1)))
#   define _DMATX_IFG       (0xF << (4*(4-1)))
#   define __UART_ISR       platform_isr_usart1
#   define __DMARX_ISR      platform_isr_dma1ch5
#   define __DMATX_ISR      platform_isr_dma1ch4
#   define __UART_CLKON()   (RCC->APB2ENR |= RCC_APB2ENR_USART1EN)
#   define __UART_CLKOFF()  (RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN)

#elif (MPIPE_UART_ID == 2)
#   define _UARTCLK         (((ot_u32)platform_ext.cpu_khz*1000)/BOARD_PARAM_APB1CLKDIV)
#   define _UARTCLK_HS      (PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB1CLKDIV)
#   define _UART_IRQ        USART2_IRQn
#   define _DMARX           DMA1_Channel6
#   define _DMATX           DMA1_Channel7
#   define _DMARX_IRQ       DMA1_Channel6_IRQn
#   define _DMATX_IRQ       DMA1_Channel7_IRQn
#   define _DMARX_IFG       (0xF << (4*(6-1)))
#   define _DMATX_IFG       (0xF << (4*(7-1)))
#   define __UART_ISR       platform_isr_usart2
#   define __DMARX_ISR      platform_isr_dma1ch6
#   define __DMATX_ISR      platform_isr_dma1ch7
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
#   define _DMARX_IFG       (0xF << (4*(3-1)))
#   define _DMATX_IFG       (0xF << (4*(2-1)))
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

#define __UART_TXOPEN() do { \
            MPIPE_UART->CR1 = 0; \
            MPIPE_UART->SR  = 0; \
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_TE); \
        } while (0)

#define __UART_RXOPEN() do { \
            MPIPE_UART->CR1 = 0; \
            MPIPE_UART->SR  = 0; \
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_RE);   \
        } while (0)
        
#define __UART_CLOSE()  (MPIPE_UART->CR1 = 0)
#define __UART_CLEAR()  (MPIPE_UART->SR  = 0)


// DMA basic control
#define __DMA_TXOPEN(SRC, SIZE) do { \
            _DMATX->CCR     = 0;                \
            _DMATX->CMAR    = (uint32_t)SRC;    \
            _DMATX->CNDTR   = (ot_u16)SIZE;     \
            DMA1->IFCR      = (_DMARX_IFG | _DMATX_IFG);       \
            _DMATX->CCR     = (DMA_CCR1_DIR | DMA_CCR1_MINC | DMA_CCR1_PL_HI | DMA_CCR1_TCIE | DMA_CCR1_EN); \
        } while (0)

#define __DMA_RXOPEN(DEST, SIZE) do { \
            _DMARX->CCR     = 0;                \
            _DMARX->CMAR    = (ot_u32)DEST;     \
            _DMARX->CNDTR   = (ot_u16)SIZE;     \
            DMA1->IFCR      = (_DMARX_IFG | _DMATX_IFG);       \
            _DMARX->CCR     = (DMA_CCR1_MINC | DMA_CCR1_PL_HI | DMA_CCR1_TCIE | DMA_CCR1_EN); \
        } while (0)        
        
#define __DMA_RX_CLOSE()    (_DMATX->CCR = 0)
#define __DMA_TX_CLOSE()    (_DMATX->CCR = 0)
#define __DMA_ALL_CLOSE()   (_DMATX->CCR = 0)
#define __DMA_RX_CLEAR()    (DMA1->IFCR = _DMARX_IFG)
#define __DMA_TX_CLEAR()    (DMA1->IFCR = _DMATX_IFG)
#define __DMA_ALL_CLEAR()   (DMA1->IFCR = (_DMARX_IFG | _DMATX_IFG))
#define __DMA_CLKON();
#define __DMA_CLKOFF();     
//#define __DMA_CLKON()       (RCC->AHBLPENR  |= RCC_AHBLPENR_DMA1LPEN)
//#define __DMA_CLKOFF()      (RCC->AHBLPENR &= ~RCC_AHBLPENR_DMA1LPEN)

#if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
#   define __SET_RTS()      (MPIPE_RTS_PORT->BSRRL = MPIPE_RTS_PIN)
#   define __CLR_RTS()      (MPIPE_RTS_PORT->BSRRH = MPIPE_RTS_PIN)
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Null)
#else
#   define __SET_RTS();
#   define __CLR_RTS();
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Idle)
#endif

// Multimode RTS clearing strategy
//#   if ((MPIPE_UARTMODES == 1) && BOARD_FEATURE_MPIPE_FLOWCTL)
//        MPIPE_UART_PORT->BSRRH = MPIPE_RTS_PIN;
//#   elif (MPIPE_UARTMODES > 1)
//        MPIPE_UART_PORT->BSRRH = (uart.mode & 2) << (MPIPE_RTS_PIN-1);
//#   endif





/** MPipe Macros 
  * ========================================================================<BR>
  */

/// @todo lookup actual data rate in use
#define __MPIPE_TIMEOUT(BYTES)  (10 + (BYTES >> 3))




/** Mpipe Driver Data  <BR>
  * ========================================================================<BR>
  */
  
typedef struct {
    ot_uni16 seq;
#if (MPIPE_USE_ACKS)
    mpipe_priority  priority;
#endif
#if (MPIPE_UARTMODES > 1)
    ot_u16 mode;
#endif
#if (BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED)
    ot_u16 counter;
#endif
} uart_struct;


uart_struct uart;


///@note the MANT+FRAC baud rate selection can be achieved by the div16 term
static const ot_u16 br_hssel[4] = { 
        (_UARTCLK_HS / (9600) ), 
        (_UARTCLK_HS / (28800) ), 
        (_UARTCLK_HS / (57600) ), 
        (_UARTCLK_HS / (115200) )
    };


  


/** Mpipe ISRs  <BR>
  * ========================================================================<BR>
  * There are two kinds of ISRs in this MPIPE driver: DMA ISRs and Pin ISRs.
  * The DMA ISRs occur when the data is finished being transmitted or received.
  * The Pin ISRs occur when there is a wake-up or handshaking signal detected.
  * 
  * CTS/RTS ISR: In RTS/CTS mode, the MPipe connection is disabled whenever the 
  * session is over, thereby setting mpipe.state to Null.  The kernel can make 
  * power optimizations if it knows MPipe is disconnected.  Of course, it also
  * provides traditional Null-modem RTS/CTS flow control.
  *
  * DTR ISR: In CS mode, the DTR line is used to wakeup this device before the 
  * host sends some data.  It allows this device to stay in deep low-power modes, 
  * and/or it allows multi-drop UART, which can be nice for sharing UART of 
  * devices like Arduino.  The DTR trigger must occur *at least 20us* before the 
  * start bit in order to guarantee the STM32L wakes-up properly and can receive 
  * the data.
  *
  * BREAK ISR: In break mode, there is a "break" character placed before the data.
  * It is used in the same way as DTR, except there is no additional line, just the 
  * normal RX line that is attached to an edge interrupt.  it uses the normal RX  
  * line instead of an additional CS/DTR line.  You can also safely use a 0x00 
  * character instead of a break.
  * 
/// 
    
  * These are DMA ISRs.  They are macros which are defined in this file, above.
  * In STM32, each DMA channel has its own IRQ, and each peripheral has mapping
  * to different channels.
  */
  
void sub_mpipe_close();
void sub_txndef(ot_bool blocking, mpipe_priority data_priority);
void sub_rxndef(ot_bool blocking, mpipe_priority data_priority);
void sub_txopen(ot_u8* data, ot_u16 length);


void sub_mpipe_close() {
    __DMA_ALL_CLOSE();
    __DMA_ALL_CLEAR();
    __DMA_CLKOFF();
    __UART_CLOSE();
    __UART_CLEAR();
    __UART_CLKOFF();
    __CLR_RTS();
    __CLR_MPIPE();
    __DISMISS_FULL_SPEED();
    
    mpipe.state = MPIPE_Idle;
}



/// CTS and DTR/CS use the same line, although the ISR behavior is different
#if ((BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED) && (BOARD_FEATURE(MPIPE_CS) != ENABLED))
void __CTS_ISR(void) {
    mpipe.state = MPIPE_Idle;
    if ((MPIPE_UART_PORT->ODR & BOARD_UART_RTSPIN) == 0) {
        sub_txndef(False, 0);
    }
    else {
        sub_rxndef(False, 0);
        MPIPE_UART_PORT->BSRRL = BOARD_UART_RTSPINNUM; 
    }
}
#elif ((BOARD_FEATURE(MPIPE_FLOWCTL) != ENABLED) && (BOARD_FEATURE(MPIPE_CS) == ENABLED))
void __CTS_ISR(void) {
    if (MPIPE_UART_PORT->IDR & BOARD_UART_CTSPIN) {
        mpipe.state = MPIPE_Null;
        mpipe_kill();
    }
    else {
        mpipe.state = MPIPE_Idle;
        sub_rxndef(False, 0);
    }
}
#elif ((BOARD_FEATURE(MPIPE_FLOWCTL) == ENABLED) && (BOARD_FEATURE(MPIPE_CS) == ENABLED))
void __CTS_ISR(void) {
    ot_u16 rts_mask;
    ot_u16 dtr_mask;
    rts_mask    = (uart.mode & 1) << BOARD_UART_RTSPIN;
    dtr_mask    = ((uart.mode-1) & 1) << BOARD_UART_CTSPIN; 
    mpipe.state = MPIPE_Idle;

    if (MPIPE_UART_PORT->IDR & dtr_mask) {
        mpipe.state--;
        mpipe_kill();
    }
    else if ((MPIPE_UART_PORT->ODR & BOARD_UART_RTSPIN) == 0) {
        sub_txndef(False, 0);
    }
    else {
        sub_rxndef(False, 0);
        MPIPE_UART_PORT->BSRRL = rts_mask;
    }

}

#endif


void __DMARX_ISR(void) {
    mpipedrv_isr();
}

void __DMATX_ISR(void) {
    mpipedrv_isr();
}


///@todo the UART TC interrupt just does not seem to work properly on STM32L.
/// Padding the TX with 2 bytes is also not sufficient because there are 
/// timing inconsistencies with the DMA<-->UART signalling interface.  
/// Instead, we need to use the ugly approach of waiting for each extra byte
/// to finish.
//void __UART_ISR(void) {
//}






/** Mpipe Main Subroutines   <BR>
  * ========================================================================
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
    ot_u8 baud_id;
    
#   if (MPIPE_UARTMODES == 1)
#       if (BOARD_PARAM_MPIPE_FLOWCTL || BOARD_PARAM_MPIPE_CS)
        mpipe.state = MPIPE_Null;
#       else
        mpipe.state = MPIPE_Idle;
#       endif
        baud_id     = MPIPE_115200bps;
#   else
        uart.mode   = ((ot_u8*)port_id)[0];
        mpipe.state = (uart.mode & 2) ? MPIPE_Null : MPIPE_Idle; 
        baud_id     = (uart.mode == 0) ? 0 : ((ot_u8*)port_id)[1];  
#   endif
    
    
    /// UART Setup (RX & TX setup takes place at time of startup)
    __UART_CLKON();
    MPIPE_UART->BRR     = br_hssel[baud_id];
    MPIPE_UART->CR3     = USART_CR3_DMAR | USART_CR3_DMAT;
    MPIPE_UART->CR2     = 0;
    MPIPE_UART->CR1     = 0;
    __UART_CLKOFF();
    
    
    /// Set up DMA channels for RX and TX
//  _DMARX->CCR     = 0;
//  _DMATX->CCR     = 0;
//  _DMARX->CNDTR   = 0;                    //buffer size, filled on usage
//  _DMATX->CNDTR   = 0;                    //buffer size, filled on usage
//  _DMARX->CMAR    = (uint32_t)(NULL);     //data buffer, filled on usage
//  _DMATX->CMAR    = (uint32_t)(NULL);     //data buffer, filled on usage
    _DMARX->CPAR    = (uint32_t)&(MPIPE_UART->DR);
    _DMATX->CPAR    = (uint32_t)&(MPIPE_UART->DR);
    
    /// MPipe RX DMA Interrupt, always used
    NVIC->IP[(ot_u32)_DMARX_IRQ]        = _IRQGROUP;
    NVIC->ISER[(ot_u32)(_DMARX_IRQ>>5)] = (1 << ((ot_u32)_DMARX_IRQ & 0x1F));
    
    NVIC->IP[(ot_u32)_DMATX_IRQ]        = _IRQGROUP;
    NVIC->ISER[(ot_u32)(_DMATX_IRQ>>5)] = (1 << ((ot_u32)_DMATX_IRQ & 0x1F));
    
    /// MPipe TX TC USART Interrupt (doesn't seem to work)
    //NVIC->IP[(ot_u32)_UART_IRQ]         = _IRQGROUP;
    //NVIC->ISER[(ot_u32)(_UART_IRQ>>5)]  = (1 << ((ot_u32)_UART_IRQ & 0x1F));
    
    
    /// Configure optional double-edge CTS interrupt
    /// NVIC configuration must be in the platform_STM32L1xx.c implementation,
    /// due to the way EXTIs are shared.
#   if (BOARD_FEATURE(MPIPE_FLOWCTL) || BOARD_FEATURE(MPIPE_CS))
        EXTI->FTSR  |= MPIPE_UART_CTSPIN;
        EXTI->RTSR  |= MPIPE_UART_CTSPIN;
#   endif
    
        
    /// Final MPIPE configuration
#   if (MPIPE_USE_ACKS)
    uart.priority   = MPIPE_Low;
#   endif
    uart.seq.ushort = 0;          //not actually necessary

    alp_init(&mpipe.alp, &dir_in, &dir_out);
    mpipe.alp.inq->back    -= 10;
    mpipe.alp.outq->back   -= 10;
        
    
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
    // Close down the peripheral
    sub_mpipe_close();
    
    // Clear the TX queue
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



#ifndef EXTF_mpipedrv_txndef
ot_uint mpipedrv_txndef(ot_bool blocking, mpipe_priority data_priority) {
/// Data TX will only occur if this function is called when the MPipe state is
/// idle.  The exception is when the function is called with ACK priority, in
/// which case the state doesn't need to be Idle.  Lastly, if you specify the 
/// blocking parameter, the function will not return until the packet is 
/// completely transmitted.
    
#   if (MPIPE_UARTMODES == 1) && (BOARD_FEATURE(MPIPE_DIRECT))
    /// Direct UART Only
    sub_txndef(blocking, data_priority);
    
#   elif (MPIPE_UARTMODES == 1) && (BOARD_FEATURE(MPIPE_CS))
    /// DTR UART Only
    if ((MPIPE_UART_PORT->IDR & MPIPE_UART_DTRPIN) == 0) {
        sub_txndef(blocking, data_priority);
    }

#   elif (MPIPE_UARTMODES == 1) && (BOARD_FEATURE(FLOW_CTL))
    /// CTS/RTS UART Only
    if ((MPIPE_UART_PORT->IDR & MPIPE_UART_CTSPIN) == 0) {
        sub_txndef(blocking, data_priority);
    }
    else {
        ///@todo implement a short timer with a guard time to prevent CTS collision.
        //disable CTS interrupt
        //counter = x;
        //while (CTS != active) {
        //counter--;
        __SET_RTS();
        //}
        //if (counter != 0) { pull down RTS, enable CTS interrupt, goto TOP }
        //else enable CTS interrupt
    }
        
#   else
    /// Multiple software configurable UART modes
    ot_u16 cts_mask;
    cts_mask    = (uart.mode & 2) << (MPIPE_UART_CTSPIN-1);
    cts_mask   &= MPIPE_UART_PORT->IDR;
    
    if (cts_mask == 0) {
        sub_txndef(blocking, data_priority);
    }
    else if (uart.mode & 1) {
        ///@todo implement a short timer with a guard time to prevent CTS collision.
        //disable CTS interrupt
        //counter = x;
        //while (CTS != active) {
        //counter--;
        __SET_RTS();
        //}
        //if (counter != 0) { pull down RTS, enable CTS interrupt, goto TOP }
        //else enable CTS interrupt
    }
#   endif

    return __MPIPE_TIMEOUT(mpipe.alp.outq->length);
}
#endif


void sub_txndef(ot_bool blocking, mpipe_priority data_priority) {
    ot_u16 length;
    ot_u8* data;

#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack)) {
        uart.priority  = data_priority;
    
        /// Todo: load the ACK
        goto mpipedrv_txndef_GO;
    }
#endif
    
    // Write the footer (sequence + CRC) onto this packet
    q_writeshort(mpipe.alp.outq, uart.seq.ushort++);
    length = mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor;
    q_writeshort(   mpipe.alp.outq, 
                    platform_crc_block(mpipe.alp.outq->getcursor, length)  );
    
    // Save the beginning of the packet (data), and move getcursor past 
    // this packet.  If another packet is queued during this transmission, 
    // it will hold its place in the queue and get piggybacked.
    length                     += (2 + 2);                      // CRC16 + Padded bytes
    data                        = mpipe.alp.outq->getcursor;
    mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;
    
    // If the transmitter is idle, start it up.
    if (mpipe.state == MPIPE_Idle) {
        mpipe.state = MPIPE_Tx_Done;    //MPIPE_Tx_Wait;
        __REQUEST_FULL_SPEED();
        
        mpipedrv_txndef_GO:
        sub_txopen(data, length);
        
        if (blocking) {
           mpipedrv_wait();
        }
    }
}



void sub_txopen(ot_u8* data, ot_u16 length) {
    __UART_CLKON();
    __UART_TXOPEN();
    __DMA_TXOPEN(data, length);
}






#ifndef EXTF_mpipedrv_rxndef
void mpipedrv_rxndef(ot_bool blocking, mpipe_priority data_priority) {
#if (MPIPE_UARTMODES == 1) && (BOARD_FEATURE(MPIPE_DIRECT))
    /// DIRECT UART Only
    sub_rxndef(blocking, data_priority);

#elif (MPIPE_UARTMODES == 1)
    /// CTS/RTS or DTR UART Only
    if (__UART_CHECKPIN())  sub_rxndef(blocking, data_priority);
    else                    EXTI->IMR |= MPIPE_CTS_PIN;

#else
    /// Multiple, software configurable UART modes
    ot_u16 cts_mask;
    cts_mask    = (uart.mode & 2) << (MPIPE_UART_CTSPIN-1);
    cts_mask   &= MPIPE_UART_PORT->IDR;
    EXTI->IMR  |= cts_mask;
    
    if (cts_mask == 0) {
        mpipe.state = MPIPE_Idle;
        sub_rxndef(blocking, data_priority);
    }
}
#endif
}
#endif

void sub_rxndef(ot_bool blocking, mpipe_priority data_priority) {
#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack) {
        uart.priority  = data_priority;
        goto mpipedrv_rxndef_SETUP;
    }
#endif
    if (blocking) {
        mpipedrv_wait();
    }
    if (mpipe.state == MPIPE_Idle) {
        mpipedrv_rxndef_SETUP:
        //mpipe.state = MPIPE_RxHeader;
        __REQUEST_FULL_SPEED();
        q_empty(mpipe.alp.inq);
        //mpipe.alp.inq->back -=10;
        
        __UART_CLKON();
        __UART_RXOPEN();
        __DMA_RXOPEN(mpipe.alp.inq->front, 10);
    }
}





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

    __DMA_ALL_CLOSE();
    __DMA_ALL_CLEAR();
    
    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!
        
        case MPIPE_RxHeader: {
            ot_u8* payload_front;
            ot_int payload_len;
            mpipe.state             = MPIPE_RxPayload;
            payload_len             = mpipe.alp.inq->front[2];
            mpipe.alp.inq->length   = payload_len + 10;
            payload_front           = mpipe.alp.inq->front + 10;
            mpipe.alp.inq->back     = payload_front + payload_len;
            __DMA_RXOPEN(payload_front, payload_len);
            
            ///@todo This should be dynamic (current value relevant for 115200),
            ///      and it will need to be conditional when RTS/CTS is enabled.
            mpipeevt_rxdetect( __MPIPE_TIMEOUT(payload_len) );
        }   return;

        case MPIPE_RxPayload: {
            ot_u8* footer;
            footer                  = mpipe.alp.inq->back;
            uart.seq.ubyte[UPPER]   = *footer++;
            uart.seq.ubyte[LOWER]   = *footer;
            
            // CRC is Good (==0) or bad (!=0) Discard the packet if bad
#           if (BOARD_FEATURE_USBCONVERTER != ENABLED)
            crc_result = platform_crc_block(mpipe.alp.inq->front, mpipe.alp.inq->length);
#           endif
            
#           if (MPIPE_USE_ACKS)
            // ACKs must be used when Broadcast mode is off
            // 1. On ACKs, txndef() requires caller to choose state 
            // 2. Copy RX'ed seq number into local seq number
            // 3. Copy NACK/ACK status to 6th byte in NDEF header
            if (uart.priority != MPIPE_Broadcast) {
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

#       if (MPIPE_USE_ACKS)        
        case MPIPE_TxAck_Done:  // TX'ed an ACK
            if (mpipe.alp.outq->front[5] != 0) { // TX'ed a NACK
                mpipedrv_rxndef(False, uart.priority);
                mpipe.state = MPIPE_RxHeader;
                return;
            }
            uart.priority = MPIPE_Low;
            goto mpipedrv_isr_RXSIG;
#       endif

        //case MPIPE_Tx_Wait:
            //Do a wait or setup some interim interrupt
            //(none used here)
            //return;

#       if (MPIPE_USE_ACKS)
        case MPIPE_Tx_Done:
            //Cancel Tx_Wait interim interrupt, if any
            //(none used here)
            if (uart.priority != MPIPE_Broadcast) {
                mpipedrv_rxndef(False, MPIPE_Ack);
                mpipe.state = MPIPE_RxAck;
                return;
            }
            goto mpipedrv_isr_TXSIG;
#       endif          
        
#       if (MPIPE_USE_ACKS)        
        case MPIPE_RxAck:
            if (platform_crc_block(mpipe.alp.inq->front, 10) != 0) { //RX'ed NACK
                mpipedrv_txndef(False, uart.priority);
                return;
            }
            goto mpipedrv_isr_TXSIG;
#       endif
            
       //default: goto mpipedrv_isr_TXSIG;  //normal behavior is to fall through
    }
    
    // The TX process is complete, including any ACKing.
    // - Check the TX queue to see if there is anything queued.
    // - If yes, then piggyback the transmission
    // - If no, then close Mpipe and call txdone event handler in the MPipe Task
    mpipedrv_isr_TXSIG:
    if (mpipe.alp.outq->getcursor != mpipe.alp.outq->putcursor) {
        ot_u16 nextpkt_len;
        nextpkt_len = (mpipe.alp.outq->putcursor - mpipe.alp.outq->getcursor) + 2;
        sub_txopen( mpipe.alp.outq->getcursor, nextpkt_len);
        return;
    }
    mpipedrv_kill();
    mpipeevt_txdone(0);
    return;
    
    
    // The RX process is complete, including and ACKing.
    // - Close MPipe and call rxdone event handler from MPipe Task
    // - If RX CRC matters, then make sure to compute it.
    mpipedrv_isr_RXSIG:
    sub_mpipe_close();
#   if (BOARD_FEATURE_USBCONVERTER != ENABLED)
        mpipeevt_rxdone((ot_int)crc_result);
#   else
        mpipeevt_rxdone(0);
#   endif
    
}
#endif








#endif

