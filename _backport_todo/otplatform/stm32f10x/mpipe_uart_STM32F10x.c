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
  * @file       /otplatform/stm32f10x/mpipe_uart_STM32F10x.c
  * @author     JP Norair
  * @version    V1.0
  * @date       31 Jul 2011
  * @brief      Message Pipe (MPIPE) UART implementation for STM32F10x
  * @defgroup   MPipe (Message Pipe)
  * @ingroup    MPipe
  *
  * Baudrates supported:    9600, 115200                                        <BR>
  * Byte structure:         8N1                                                 <BR>
  * Duplex:                 Half                                                <BR>
  * Flow control:           Custom, ACK-based                                   <BR>
  * Connection:             RS-232, DTE-DTE (use a null-modem connector)        <BR><BR>
  * 
  * Implemented Mpipe Protocol:                                                 <BR>
  * The Mpipe protocol is a simple wrapper to NDEF.                             <BR>
  * Legend: [ NDEF Header ] [ NDEF Payload ] [ Seq. Number ] [ CRC16 ]          <BR>
  * Bytes:        6             <= 255             2             2              <BR><BR>
  *
  * The protocol includes an ACK/NACK feature.  After receiving a message, the  <BR>
  * Mpipe send an ACK/NACK.  The "YY" byte is 0 for ACK and non-zero for ACK.   <BR>
  * Presently, 0x7F is used as the YY NACK value.                               <BR>
  * [ Seq ID ] 0xDD 0x00 0x00 0x02 0x00 0xYY  [ CRC16 ]
  ******************************************************************************
  */


#include <otsys/config.h>
#include <otplatform.h>

// Compile only when MPipe is enabled, but USB is disabled
#if ((OT_FEATURE(MPIPE) == ENABLED) && (MCU_CONFIG(MPIPECDC) != ENABLED))

#include <otsys/mpipe.h>
#include <otlib/utils.h>



/** Speed / Baudrate Calculation constants   <BR>
  * ========================================================================<BR>
  * By OpenTag standards, APB1 is Clock/4, APB2 is Clock/1.  The baud rate 
  * generation value is (basically) based on the peripheral clock / baudrate.
  * You should tweak these if you change the bus clock dividers.
  */

#if (MPIPE_UART_INDEX == 1)
#   define UARTCLK          (PLATFORM_HSCLOCK_HZ/1)
#   define __UART_CLKON()   do { RCC->APB2ENR |= RCC_APB2Periph_USART1  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB2ENR &= ~RCC_APB2Periph_USART1 } while(0)

#elif (MPIPE_UART_INDEX == 2)
#   define UARTCLK     (PLATFORM_HSCLOCK_HZ/4)
#   define __UART_CLKON()   do { RCC->APB1ENR |= RCC_APB1Periph_USART2  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB1ENR &= ~RCC_APB1Periph_USART2 } while(0)

#elif (MPIPE_UART_INDEX == 3)
#   define UARTCLK     (PLATFORM_HSCLOCK_HZ/4)
#   define __UART_CLKON()   do { RCC->APB1ENR |= RCC_APB1Periph_USART3  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB1ENR &= ~RCC_APB1Periph_USART3 } while(0)

#elif (MPIPE_UART_INDEX == 4)
#   define UARTCLK     (PLATFORM_HSCLOCK_HZ/4)
#   define __UART_CLKON()   do { RCC->APB1ENR |= RCC_APB1Periph_UART4  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB1ENR &= ~RCC_APB1Periph_UART4 } while(0)

#elif (MPIPE_UART_INDEX == 5)
#   define UARTCLK     (PLATFORM_HSCLOCK_HZ/4)
#   define __UART_CLKON()   do { RCC->APB1ENR |= RCC_APB1Periph_UART5  } while(0)
#   define __UART_CLKOFF()  do { RCC->APB1ENR &= ~RCC_APB1Periph_UART5 } while(0)

#else
#   error "MPIPE_UART_INDEX is misdefined, must be 1 to 5"

#endif





#define DIV9600     (UARTCLK/(9600))
#define DIV28800    (UARTCLK/(28800))
#define DIV57600    (UARTCLK/(57600))
#define DIV115200   (UARTCLK/(115200))



/** Peripheral Control Macros  <BR>
  * ========================================================================<BR>
  */

// UART basic control
#define __UART_CLOSE()      (MPIPE_UART->CR1 &= ~(USART_CR1_UE | USART_CR1_TCIE))
#define __UART_TXOPEN()     (MPIPE_UART->CR1 |= (USART_CR1_UE | USART_Mode_Tx))
#define __UART_RXOPEN()     (MPIPE_UART->CR1 |= (USART_CR1_UE | USART_Mode_Rx))
#define __UART_CLEAR()      (MPIPE_UART->SR &= ~USART_SR_TC)
    

// DMA basic control
#define __DMA_RX_ON         (MPIPE_DMA_RXCHAN->CCR |= DMA_CCR1_EN)
#define __DMA_RX_OFF        (MPIPE_DMA_RXCHAN->CCR &= ~DMA_CCR1_EN)
#define __DMA_TX_ON         (MPIPE_DMA_TXCHAN->CCR |= DMA_CCR1_EN)
#define __DMA_TX_OFF        (MPIPE_DMA_TXCHAN->CCR &= ~DMA_CCR1_EN)
#define __DMA_RX(ONOFF)     __DMA_RX_##ONOFF
#define __DMA_TX(ONOFF)     __DMA_TX_##ONOFF
#define __DMA_RX_CLEAR()    (MPIPE_DMA->IFCR = MPIPE_DMA_RXINT)
#define __DMA_TX_CLEAR()    (MPIPE_DMA->IFCR = MPIPE_DMA_TXINT)
#define __DMA_ALL_CLEAR()   (MPIPE_DMA->IFCR = (MPIPE_DMA_TXINT | MPIPE_DMA_RXINT))

// TX / RX configuration of DMAs
#define __DMA_RXCONFIG(DEST, SIZE) \
    do { \
        MPIPE_DMA_RXCHAN->CMAR  = (uint32_t)DEST; \
        MPIPE_DMA_RXCHAN->CNDTR = (ot_u16)SIZE; \
    } while(0)

#define __DMA_TXCONFIG(SRC, SIZE) \
    do { \
        MPIPE_DMA_TXCHAN->CMAR  = (uint32_t)SRC; \
        MPIPE_DMA_TXCHAN->CNDTR = (ot_u16)SIZE; \
    } while(0)  

// Software triggering of DMAs
#define __DMA_TXTRIGGER()   do { \
                                __UART_CLEAR(); \
                                __DMA_TX(ON);   \
                                MPIPE_UART->CR1 |= USART_CR1_TCIE; \
                            } while(0)
                        
#define __DMA_RXTRIGGER()   __DMA_RX(ON)





/** MPIPE Embedded ISRs  <BR>
  * ========================================================================<BR>
  * In the platform_config_~.h file, you should set the MPIPE to have embedded
  * ISRs if you want them to be included in this file.  In this case, they are
  * modularized from the rest of your code (which can be nice), but you can't
  * use the ISRs for other things.
  */

#if (ISR_EMBED(MPIPE) == ENABLED)
#   if ((MPIPE_DMA_RXINDEX == 11) || (MPIPE_DMA_TXINDEX == 11))
        void DMA1_Channel1_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 12) || (MPIPE_DMA_TXINDEX == 12))
        void DMA1_Channel2_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 13) || (MPIPE_DMA_TXINDEX == 13))
        void DMA1_Channel3_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 14) || (MPIPE_DMA_TXINDEX == 14))
        void DMA1_Channel4_IRQHandler(void)     { 
            mpipe_isr(); 
        }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 15) || (MPIPE_DMA_TXINDEX == 15))
        void DMA1_Channel5_IRQHandler(void)     { 
            mpipe_isr(); 
        }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 16) || (MPIPE_DMA_TXINDEX == 16))
        void DMA1_Channel6_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 17) || (MPIPE_DMA_TXINDEX == 17))
        void DMA1_Channel7_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 21) || (MPIPE_DMA_TXINDEX == 21))
        void DMA2_Channel1_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 22) || (MPIPE_DMA_TXINDEX == 22))
        void DMA2_Channel2_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 23) || (MPIPE_DMA_TXINDEX == 23))
        void DMA2_Channel3_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 24) || (MPIPE_DMA_TXINDEX == 24))
        void DMA2_Channel4_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if ((MPIPE_DMA_RXINDEX == 25) || (MPIPE_DMA_TXINDEX == 25))
        void DMA2_Channel5_IRQHandler(void)     { mpipe_isr(); }
#   endif
#endif



#if (ISR_EMBED(MPIPE) == ENABLED)
#   if (MPIPE_UART_INDEX == 1)
        void USART1_IRQHandler(void)    { 
            mpipe_isr(); 
        }
#   endif
#   if (MPIPE_UART_INDEX == 2)
        void USART2_IRQHandler(void)    { mpipe_isr(); }
#   endif
#   if (MPIPE_UART_INDEX == 3)
        void USART3_IRQHandler(void)    { mpipe_isr(); }
#   endif
#   if (MPIPE_UART_INDEX == 4)
        void UART4_IRQHandler(void)     { mpipe_isr(); }
#   endif
#   if (MPIPE_UART_INDEX == 5)
        void UART5_IRQHandler(void)     { mpipe_isr(); }
#   endif
#endif





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
alp_tmp         mpipe_alp;









/** Mpipe Main Subroutines   <BR>
  * ========================================================================
  */
void sub_uart_portsetup() {
    /// Remap MPIPE UART port to its alternate setting (if specified)
#   if (MPIPE_REMAP == ENABLED)
        GPIO_PinRemapConfig(MPIPE_REMAP_TYPE, ENABLE);
#   endif
    
    /// Configure UART RX & TX pins as in-floating and out-push/pull
    ///@todo optimize using registers only
    {
        GPIO_InitTypeDef GPIOinit;
        
        GPIOinit.GPIO_Speed = GPIO_Speed_50MHz;
        GPIOinit.GPIO_Pin   = MPIPE_RXD_PIN;
        GPIOinit.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_Init(MPIPE_RXD_PORT, &GPIOinit);
        
        GPIOinit.GPIO_Pin   = MPIPE_TXD_PIN;
        GPIOinit.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_Init(MPIPE_TXD_PORT, &GPIOinit);
    }

    
    /// MPIPE UART Interrupt: 0,3 priority (high)
    NVIC->IP[(ot_u32)MPIPE_UART_IRQn]           = b0011 << (8 - __NVIC_PRIO_BITS);
    NVIC->ISER[(ot_u32)(MPIPE_UART_IRQn>>5)]    = (1 << ((ot_u32)MPIPE_UART_IRQn & 0x1F));

    /// MPIPE DMA RX & TX interrupts get 0,3 priority (high)
    NVIC->IP[(ot_u32)MPIPE_DMA_RXIRQn]          = b0011 << (8 - __NVIC_PRIO_BITS);
    NVIC->IP[(ot_u32)MPIPE_DMA_TXIRQn]          = b0011 << (8 - __NVIC_PRIO_BITS);
    NVIC->ISER[(ot_u32)(MPIPE_DMA_RXIRQn>>5)]   = (1 << ((ot_u32)MPIPE_DMA_RXIRQn & 0x1F));
    NVIC->ISER[(ot_u32)(MPIPE_DMA_TXIRQn>>5)]   = (1 << ((ot_u32)MPIPE_DMA_RXIRQn & 0x1F));

    /// Set up DMA channels for RX and TX
    MPIPE_DMA_RXCHAN->CCR   =   DMA_DIR_PeripheralSRC       | \
                                DMA_Mode_Normal             | \
                                DMA_PeripheralInc_Disable   | \
                                DMA_MemoryInc_Enable        | \
                                DMA_PeripheralDataSize_Byte | \
                                DMA_MemoryDataSize_Byte     | \
                                DMA_Priority_VeryHigh       | \
                                DMA_M2M_Disable             | \
                                DMA_IT_TC;
    
    MPIPE_DMA_TXCHAN->CCR   =   DMA_DIR_PeripheralDST       | \
                                DMA_Mode_Normal             | \
                                DMA_PeripheralInc_Disable   | \
                                DMA_MemoryInc_Enable        | \
                                DMA_PeripheralDataSize_Byte | \
                                DMA_MemoryDataSize_Byte     | \
                                DMA_Priority_VeryHigh       | \
                                DMA_M2M_Disable             | \
                                DMA_IT_TC;
    
//  MPIPE_DMA_RXCHAN->CNDTR = 0;                    //buffer size, filled on usage
//  MPIPE_DMA_TXCHAN->CNDTR = 0;                    //buffer size, filled on usage
//  MPIPE_DMA_RXCHAN->CMAR  = (uint32_t)(NULL);     //data buffer, filled on usage
//  MPIPE_DMA_TXCHAN->CMAR  = (uint32_t)(NULL);     //data buffer, filled on usage
    MPIPE_DMA_RXCHAN->CPAR  = (uint32_t)&MPIPE_UART->DR;
    MPIPE_DMA_TXCHAN->CPAR  = (uint32_t)&MPIPE_UART->DR;
    
    /// UART Setup (RX & TX setup takes place at time of startup)
    __UART_CLKON();
    MPIPE_UART->CR1 = USART_WordLength_8b | USART_Parity_No;  
    MPIPE_UART->CR2 = USART_StopBits_1;
    MPIPE_UART->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
    MPIPE_UART->GTPR= 0;
}






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
#ifndef EXT_mpipe_footerbytes
ot_u8 mpipe_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif


#ifndef EXT_mpipe_init
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
    mpipe_alp.inq   = &otmpin;
    mpipe_alp.outq  = &otmpout;

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


#ifndef EXT_mpipe_wait
void mpipe_wait() {
    while (mpipe.state != MPIPE_Idle) {
        SLEEP_MCU();
    }
}
#endif


#ifndef EXT_mpipe_setspeed
void mpipe_setspeed(mpipe_speed speed) {
    static const ot_u16 br_lut[4] = { DIV9600, DIV28800, DIV57600, DIV115200 };
    MPIPE_UART->BRR = br_lut[(ot_u8)speed];
}
#endif


#ifndef EXT_mpipe_status
mpipe_state mpipe_status() {
    return mpipe.state;
}
#endif


#ifndef EXT_mpipe_tx
void mpipe_tx(ot_u8* data, ot_bool blocking, mpipe_priority data_priority) {
    ot_uni16 crcval;
    ot_int data_length;
    
#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack)) {
        mpipe.priority  = data_priority;
        goto mpipe_tx_SETUP;
    }
#endif
    if (mpipe.state == MPIPE_Idle) {
        mpipe.state     = MPIPE_Tx_Wait
        mpipe_tx_SETUP:
        mpipe.pktbuf    = data;
        data_length     = data[2] + 6;
        //mpipe.pktlen    = data[2] + 6;
    
        // add sequence id & crc to end of the datastream
        data[data_length++] = mpipe.sequence.ubyte[UPPER];
        data[data_length++] = mpipe.sequence.ubyte[LOWER];
        crcval.ushort       = crc16drv_block(data, data_length);
        data[data_length++] = crcval.ubyte[UPPER];
        data[data_length++] = crcval.ubyte[LOWER];
        
        __DMA_TX(OFF);
        __DMA_TXCONFIG(data, data_length);
        __UART_TXOPEN();
        __DMA_TXTRIGGER();
    
        if (blocking == True) {
            mpipe_wait();	
        }
    }
}
#endif


#ifndef EXT_mpipe_rxndef
ot_int mpipe_rxndef(ot_u8* data, ot_bool blocking, mpipe_priority data_priority) {
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
    
        __DMA_RX(OFF); 
        __DMA_RXCONFIG(data, 10);
        __UART_RXOPEN();
        __DMA_RXTRIGGER();
    }
}
#endif



#ifndef EXT_mpipe_isr
void mpipe_isr() {
    __DMA_RX(OFF);
    __DMA_TX(OFF);
    __DMA_ALL_CLEAR();
    
    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!
#           if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_rxdetect))
                mpipe.sig_rxdetect(0);  
#           elif defined(EXTF_mpipe_sig_rxdetect)
                mpipe_sig_rxdetect(0);
#           endif
        
        case MPIPE_RxHeader: {
            mpipe.state     = MPIPE_RxPayload;
            mpipe.pktlen   += mpipe.pktbuf[2] + MPIPE_FOOTERBYTES;
            //@todo Might need to put in a DMA Disabler here
            __DMA_RXCONFIG(mpipe.pktbuf+6, mpipe.pktlen-6);
            return;
        }
        
        case MPIPE_RxPayload: 
#           if (MPIPE_USE_ACKS)
            // ACKs must be used when Broadcast mode is off
            if (mpipe.priority != MPIPE_Broadcast) {
                // 1. On ACKs, tx() requires caller to choose state 
                // 2. Copy RX'ed sequence number into local sequence number
                // 3. Copy NACK/ACK status to 6th byte in NDEF header
                {
                    ot_u8* scratch;
                    mpipe.state                 = MPIPE_TxAck_Wait;
                    scratch                     = &mpipe.pktbuf[mpipe.pktlen-MPIPE_FOOTERBYTES];
                    mpipe.sequence.ubyte[UPPER] = *scratch++;
                    mpipe.sequence.ubyte[LOWER] = *scratch;
                }
                mpipe.ackbuf[5] = (crc16drv_block(mpipe.pktbuf, mpipe.pktlen)) ? 0x7F : 0;
                mpipe_tx(mpipe.ackbuf, False, MPIPE_Ack);
                return;
            }
#           endif
            goto mpipe_isr_RXDONE;
        
        case MPIPE_TxAck_Wait:
            // There is still data stuck in the UART.  The next interrupt will 
            // be UART TC, which uses the same ISR as DMA.
            mpipe.state++;
            return;
        
        case MPIPE_TxAck_Done: 
#           if (MPIPE_USE_ACKS)
            if (mpipe.ackbuf[5] != 0) { // TX'ed a NACK
                mpipe_rxndef(mpipe.pktbuf, False, mpipe.priority);
                mpipe.state = MPIPE_RxHeader;
                return;
            }
#           endif
            goto mpipe_isr_RXDONE;

        
        case MPIPE_Tx_Wait:
            // There is still data stuck in the UART.  The next interrupt will 
            // be UART TC, which uses the same ISR as DMA.
            mpipe.state++;
            return;
        
        case MPIPE_Tx_Done: 
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
            if (crc16drv_block(mpipe.ackbuf, 10) != 0) { //RX'ed NACK
                mpipe_tx(mpipe.pktbuf, False, mpipe.priority);
                break;
            }
#           endif
            goto mpipe_isr_TXDONE;  //RX'ed ACK
            
       default: mpipe_kill();
                return;
    }
    
    
    // This is a stack-less RX-Done subroutine
    mpipe_isr_RXDONE:
    __UART_CLOSE()
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
    __UART_CLEAR();
    __UART_CLOSE();
    mpipe.state = MPIPE_Idle;
#   if (MPIPE_USE_ACKS)
        mpipe.priority = MPIPE_Low;
#   endif
    mpipe.sequence.ushort++;    //increment sequence on TX Done
#   if ((OT_FEATURE(MPIPE_CALLBACKS) == ENABLED) && !defined(EXTF_mpipe_sig_txdone))
        mpipe.sig_txdone(0);
#   elif defined(EXTF_mpipe_sig_txdone)
        mpipe_sig_txdone(0);
#   endif
}
#endif


#endif

