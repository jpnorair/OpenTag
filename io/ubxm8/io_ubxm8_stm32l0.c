/* Copyright 2017 JP Norair
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
  * @file       /hbuilder/ubxm8/io_ubxm8_stm32l0.c
  * @author     JP Norair
  * @version    R103
  * @date       21 April 2017
  * @brief      UBX UART implementation for STM32L0xx
  * @defgroup   UBXM8
  * @ingroup    UBXM8
  *
  * The UBX protocol is to communication with uBlox GPS Modules.  This 
  * implementation only supports, officially, the M8 series.  It may also 
  * support others.
  *
  * UBX Protocol
  * <PRE>
  * +-------+-----------+-------+-----------+----------+---------+---------+
  * | Field | Sync Word | CRC16 | P. Length | Sequence | Control | Payload |
  * | Bytes |     2     |   2   |     2     |     1    |    1    |    N    |
  * | Value |   FF55    |       |     N     |   0-255  |   RFU   |   ALP   |
  * +-------+-----------+-------+-----------+----------+---------+---------+
  * </PRE>
  *
  * 
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>

#define UBXDRV_ENABLED          (BOARD_FEATURE(UBX_GNSS))
#define THIS_UBXDRV_SUPPORTED   1

#if (defined(__STM32L0__) && OT_FEATURE(GNSS) && UBXDRV_ENABLED && THIS_UBXDRV_SUPPORTED)

#include <otlib/buffers.h>
#include <otlib/delay.h>

#include <io/ubxm8/ubx_gnss.h>        
#include <otsys/sysclock.h>


#include "ubx_internal.h"

extern ubx_module_t ubx;




/** UBX UART Protocol speed configuration <BR>
  * ========================================================================<BR>
  * Default baud is 9600.  
  * Baud can be set to a faster rate after initialization.
  */
#define UBXBAUD_STARTUP        9600
#define UBXBAUD_OPERATION      115200

/// to get low-power operation, you should enable this.
#define __USE_RXBREAK          0



#if ((BOARD_FEATURE(UBX) == ENABLED) && (OT_FEATURE(UBX) == DISABLED))
#   warning "UBX is enabled in your board config but not your app config.  Usually this doesn't work."
#endif

#define UBX_HEADERBYTES   4
#define UBX_FOOTERBYTES   2
#define UBX_DMAFLUFF      0

#define UBX_OVERHEADBYTES (UBX_HEADERBYTES + UBX_FOOTERBYTES)



/** UBX Interrupt Configuration  <BR>
  * ========================================================================<BR>
  * NVIC parameters are negotiated or defaulted, considering the preset config
  * in the platform section.
  */
#ifndef BUILD_NVIC_SUBGROUP_UBX
#   define BUILD_NVIC_SUBGROUP_UBX 0
#endif
#define _IRQGROUP   (PLATFORM_NVIC_IO_GROUP)





/** Platform Clock Configuration   <BR>
  * ========================================================================<BR>
  * UART typically requires the system clock to be set to a certain speed in
  * order for the baud rate generation to work.  So, platforms that are using
  * multispeed clocking will need some extra logic in the UART driver to
  * assure that the clock speed is on the right setting during UART usage.
  */
#if MCU_CONFIG(MULTISPEED)
#   if BOARD_FEATURE(FULLSPEED)
#       define __SYS_CLKON()    (ubx.clkhandle = sysclock_request(SPEED_Full))
#       define __SYS_CLKOFF()   sysclock_dismiss(ubx.clkhandle)
#   elif BOARD_FEATURE(FLANKSPEED)
#       define __SYS_CLKON()    (ubx.clkhandle = sysclock_request(SPEED_Flank))
#       define __SYS_CLKOFF()   sysclock_dismiss(ubx.clkhandle)
#   else
#       warning "MSI oscillator (Std Speed) is not usually precise enough to run UART."
#       define __SYS_CLKON();
#       define __SYS_CLKOFF();
#   endif
#else
#   if ((BOARD_FEATURE(FULLSPEED) != ENABLED) && (BOARD_FEATURE(FLANKSPEED) != ENABLED))
#       warning "MSI oscillator (Std Speed) is not usually precise enough to run UART."
#   endif
#       define __SYS_CLKON();
#       define __SYS_CLKOFF();
#endif



/** UBX Peripheral Mapping  <BR>
  * ========================================================================<BR>
  * The STM32L0 can support the following channels for UART DMA:
  * UART1 TX:   Ch2 or Ch4
  * UART1 RX:   Ch3 or Ch5
  * UART2 TX:   Ch4 or Ch7
  * UART2 RX:   Ch5 or Ch6
  *
  * It is recommended to use Ch4 & Ch5, because most STM32L0 devices do not
  * have I2C2 (which shares these channels), because SPI1 needs Ch2 & Ch3, and
  * because SPI2 can be mapped onto Ch6 & Ch7.
  */
#if (GNSS_UART_ID == 1)
#   if (GNSS_DMA_RXCHAN_ID == 5)
#       define _DMARX       DMA1_Channel5
#       define __DMARX_ISR  platform_isr_dma1ch5
#       define _DMARX_IRQ   DMA1_Channel4_5_6_7_IRQn
#   elif (GNSS_DMA_RXCHAN_ID == 3)
#       define _DMARX       DMA1_Channel3
#       define __DMARX_ISR  platform_isr_dma1ch3
#       define _DMARX_IRQ   DMA1_Channel2_3_IRQn
#   else
#       error "GNSS RX DMA not configured on channel 3 or 5."
#   endif
#   if (GNSS_DMA_TXCHAN_ID == 4)
#       define _DMATX       DMA1_Channel4
#       define __DMATX_ISR  platform_isr_dma1ch4
#       define _DMATX_IRQ   DMA1_Channel4_5_6_7_IRQn
#   elif (GNSS_DMA_TXCHAN_ID == 2)
#       define _DMATX       DMA1_Channel2
#       define __DMATX_ISR  platform_isr_dma1ch2
#       define _DMATX_IRQ   DMA1_Channel2_3_IRQn
#   else
#       error "GNSS TX DMA not configured on channel 2 or 4."
#   endif
#   define _UART_IRQ        USART1_IRQn

#   define _DMARX_IFG       (0xF << (4*(GNSS_DMA_RXCHAN_ID-1)))
#   define _DMATX_IFG       (0xF << (4*(GNSS_DMA_TXCHAN_ID-1)))
#   define _DMA_CSEL_MASK   ((0xF << (4*(GNSS_DMA_RXCHAN_ID-1))) | (0xF << (4*(GNSS_DMA_TXCHAN_ID-1))))
#   define _DMA_CSEL        ((0x3 << (4*(GNSS_DMA_RXCHAN_ID-1))) | (0x3 << (4*(GNSS_DMA_TXCHAN_ID-1))))
#   define _DMARX_CSEL      (0x3 << (4*(GNSS_DMA_RXCHAN_ID-1)))
#   define _DMATX_CSEL      (0x3 << (4*(GNSS_DMA_TXCHAN_ID-1)))
#   define __UART_ISR       platform_isr_usart1
#   define __UART_CLKHZ()   platform_get_clockhz(2)
#   define __UART_CLKON()   (RCC->APB2ENR |= RCC_APB2ENR_USART1EN)
#   define __UART_CLKOFF()  (RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN)

#elif (GNSS_UART_ID == 2)
#   if (GNSS_DMA_RXCHAN_ID == 5)
#       define _DMARX       DMA1_Channel5
#       define __DMARX_ISR  platform_isr_dma1ch5
#   elif (GNSS_DMA_RXCHAN_ID == 6)
#       define _DMARX       DMA1_Channel6 
#       define __DMARX_ISR  platform_isr_dma1ch6
#   else
#       error "GNSS RX DMA not configured on channel 5 or 6."
#   endif
#   if (GNSS_DMA_TXCHAN_ID == 4)
#       define _DMATX       DMA1_Channel4
#       define __DMATX_ISR  platform_isr_dma1ch4
#   elif (GNSS_DMA_TXCHAN_ID == 7)
#       define _DMATX       DMA1_Channel7 
#       define __DMATX_ISR  platform_isr_dma1ch7
#   else
#       error "GNSS TX DMA not configured on channel 4 or 7."
#   endif
#   define _UART_IRQ        USART2_IRQn
#   define _DMARX_IRQ       DMA1_Channel4_5_6_7_IRQn
#   define _DMATX_IRQ       DMA1_Channel4_5_6_7_IRQn
#   define _DMARX_IFG       (0xF << (4*(GNSS_DMA_RXCHAN_ID-1)))
#   define _DMATX_IFG       (0xF << (4*(GNSS_DMA_TXCHAN_ID-1)))
#   define _DMA_CSEL_MASK   ((0xF << (4*(GNSS_DMA_RXCHAN_ID-1))) | (0xF << (4*(GNSS_DMA_TXCHAN_ID-1))))
#   define _DMA_CSEL        ((0x4 << (4*(GNSS_DMA_RXCHAN_ID-1))) | (0x4 << (4*(GNSS_DMA_TXCHAN_ID-1))))
#   define _DMARX_CSEL      (0x4 << (4*(GNSS_DMA_RXCHAN_ID-1)))
#   define _DMATX_CSEL      (0x4 << (4*(GNSS_DMA_TXCHAN_ID-1)))
#   define __UART_ISR       platform_isr_usart2
#   define __UART_CLKHZ()   platform_get_clockhz(1)
#   define __UART_CLKON()   (RCC->APB1ENR |= RCC_APB1ENR_USART2EN)
#   define __UART_CLKOFF()  (RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN)

//#elif (GNSS_UART_ID == 3)
// This could be the LPUART, but that is not supported at the moment

#else
#   error "GNSS_UART_ID is misdefined, must be 1, or 2"

#endif





/** Peripheral Control Macros  <BR>
  * ========================================================================<BR>
  */
#define __CLR_UBX()     (ubx.state = UBX_Null)

#define __UART_CLOSE()  (GNSS_UART->CR1 = 0)
#define __UART_CLEAR()  (GNSS_UART->ICR = (1<<20)|(1<<17)|(1<<12)|(1<<11)|(1<<9)|(1<<8)|(1<<6)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0))

#define __UART_TXOPEN() do { \
        GNSS_UART->CR1 = 0; \
        __UART_CLEAR(); \
        GNSS_UART->CR1 = (USART_CR1_UE | USART_CR1_TE); \
    } while (0)

#define __UART_RXOPEN() do { \
        GNSS_UART->CR1 = 0; \
        __UART_CLEAR(); \
        GNSS_UART->CR1 = (USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE);   \
    } while (0)


// DMA basic control
#define __DMA_TXOPEN(SRC, SIZE) do { \
        _DMATX->CCR         = 0;                \
        _DMATX->CMAR        = (uint32_t)SRC;    \
        _DMATX->CNDTR       = (ot_u16)SIZE;     \
        DMA1->IFCR          = (_DMARX_IFG | _DMATX_IFG);       \
        DMA1_CSELR->CSELR   = (DMA1_CSELR->CSELR & ~_DMA_CSEL_MASK) | _DMATX_CSEL; \
        _DMATX->CCR         = (DMA_CCR_DIR | DMA_CCR_MINC | (2<<DMA_CCR_PL_Pos) | DMA_CCR_TCIE | DMA_CCR_EN); \
    } while (0)

#define __DMA_RXOPEN(DEST, SIZE) do { \
        _DMARX->CCR         = 0;                \
        _DMARX->CMAR        = (ot_u32)DEST;     \
        _DMARX->CNDTR       = (ot_u16)SIZE;     \
        DMA1->IFCR          = (_DMARX_IFG | _DMATX_IFG);       \
        DMA1_CSELR->CSELR   = (DMA1_CSELR->CSELR & ~_DMA_CSEL_MASK) | _DMARX_CSEL; \
        _DMARX->CCR         = (DMA_CCR_MINC | (2<<DMA_CCR_PL_Pos) | DMA_CCR_TCIE | DMA_CCR_EN); \
    } while (0)

#define __DMA_RX_CLOSE()    do { DMA1_CSELR->CSELR &= ~_DMA_CSEL_MASK; _DMARX->CCR = 0; } while(0)
#define __DMA_TX_CLOSE()    do { DMA1_CSELR->CSELR &= ~_DMA_CSEL_MASK; _DMATX->CCR = 0; } while(0)
#define __DMA_ALL_CLOSE()   do { DMA1_CSELR->CSELR &= ~_DMA_CSEL_MASK; _DMATX->CCR = 0; _DMARX->CCR = 0; } while(0)
#define __DMA_RX_CLEAR()    (DMA1->IFCR = _DMARX_IFG)
#define __DMA_TX_CLEAR()    (DMA1->IFCR = _DMATX_IFG)
#define __DMA_ALL_CLEAR()   (DMA1->IFCR = (_DMARX_IFG | _DMATX_IFG))
#define __DMA_CLKON();
#define __DMA_CLKOFF();
//#define __DMA_CLKON()       (RCC->AHBLPENR  |= RCC_AHBLPENR_DMA1LPEN)
//#define __DMA_CLKOFF()      (RCC->AHBLPENR &= ~RCC_AHBLPENR_DMA1LPEN)





/** UBX/UBX Macros
  * ========================================================================<BR>
  */

#define __UBX_TIMEOUT(BYTES)  (1 + (((10485760/ubx.baudrate) * BYTES) >> 10) )




/** UBX/UBX Driver Data  <BR>
  * ========================================================================<BR>
  */
//static const ot_u16 _miti_per_byte[] = { \
//    (1093),     // 9600
//    (92),       // 115200
//    (42),       // 250000
//    (21)        // 500000
//};

//static const ot_u32 _brtable[] = { \
//    MCU_PARAM(UART_9600BPS),
//    MCU_PARAM(UART_115200BPS),
//    MCU_PARAM(UART_250000BPS),
//    MCU_PARAM(UART_500000BPS),
//};

#define __SET_USARTDIV(BAUD)    ( (__UART_CLKHZ() + (BAUD>>1)) / BAUD)


void sub_ubx_open();
void sub_ubx_close();
void sub_ubx_txcont();



void sub_ubx_open() {
    ot_u32 scratch;
    __SYS_CLKON();
    __UART_CLKON();

    scratch                 = BOARD_EXTUART_PORT->MODER;
    scratch                &= ~((3 << (BOARD_EXTUART_TXPINNUM*2)) | (3 << (BOARD_EXTUART_RXPINNUM*2)));
    scratch                |= (2 << (BOARD_EXTUART_TXPINNUM*2)) | (2 << (BOARD_EXTUART_RXPINNUM*2));
    BOARD_EXTUART_PORT->MODER  = scratch;
    //ubx.state             = UBX_Idle;
}

void sub_ubx_close() {
    ot_u32 scratch          = BOARD_EXTUART_PORT->MODER;
    scratch                &= ~((3 << (BOARD_EXTUART_TXPINNUM*2)) | (3 << (BOARD_EXTUART_RXPINNUM*2)));
    scratch                |= (1 << (BOARD_EXTUART_TXPINNUM*2)) | (0 << (BOARD_EXTUART_RXPINNUM*2));
    BOARD_EXTUART_PORT->MODER  = scratch;

    __DMA_ALL_CLOSE();
    __DMA_ALL_CLEAR();
    __DMA_CLKOFF();
    __UART_CLOSE();
    __UART_CLEAR();
    __UART_CLKOFF();

    //__DISMISS_FULL_SPEED();
    __CLR_UBX();
}


ot_u16 sub_ubx_fletcher(ot_u8* input, ot_uint length) {
    ot_uni16 ck;
    ck.ushort = 0;
    
//    for (ot_int i=0; i<length; i++) {
//        ck.ubyte[0] += input[i];
//        ck.ubyte[1] += ck.ubyte[0];
//    }
//    return ck.ushort;
    
    while (length > 0) {
        length--;
        ck.ubyte[0] += *input++;
        ck.ubyte[1] += ck.ubyte[0];
    }
    return ck.ushort;
}




/** UBX ISRs  <BR>
  * ========================================================================<BR>
  * There are two kinds of ISRs in this driver: DMA ISRs and Pin ISRs.
  * The DMA ISRs occur when the data is finished being transmitted or received.
  * The Pin ISRs occur when there is a wake-up or handshaking signal detected.
  */

void ubxdrv_rxsync_isr(void) {
#   if (__USE_RXBREAK)
    // Falling edge ISR on RX line, which can wakeup from stop
    EXTI->IMR  &= ~GNSS_UART_RXPIN;
    ubx.buf.sync.syncB5 = 0xB5;
#   else
    ubx.buf.sync.syncB5 = 0;
#   endif
    ubx.buf.sync.sync62 = 0;
    
    // Open UART for character-RX
    ubx.state       = UBX_Idle;
    //ubxevt_rxinit();        // can be used for RX timeout
    sub_ubx_open();
    __UART_RXOPEN();
}


void __UART_ISR(void) {
    ot_u32 uart_isr;
    uart_isr = GNSS_UART->ISR;
    __UART_CLEAR();
    
    /// USART interrupts that we care about:
    /// - CM:   Character match, used to detect 0x62 sync byte.  Only for initial RX.
    /// - RXNE  RX register interrupt, used as alternate implementation of Sync Match
    /// - TC    b__: Transmission Complete interrupt
    
    // char-by-char sync detect
    // Using &ubx.rxq.front[-1] eats the last sync byte

    if (uart_isr & USART_ISR_RXNE) {
        ot_u8 inbyte = GNSS_UART->RDR;
        
        if (ubx.buf.sync.syncB5 == 0xB5) {
            ubx.buf.sync.sync62 = inbyte;
            
            if (ubx.buf.sync.sync62 == 0x62) {
                ubx.state       = UBX_RxHeader;
                GNSS_UART->CR1 = (USART_CR1_UE | USART_CR1_RE);
                __DMA_RXOPEN( &ubx.buf.sync.sync62, UBX_OVERHEADBYTES+1);
                //ubxevt_rxsync(0);
            }
            else {
                ubx.buf.sync.syncB5 = 0;
                ubx.buf.sync.sync62 = 0;
            }            
        }
        else {
            ubx.buf.sync.syncB5 = inbyte;
        }
    }

    // TX Complete
    else if ((uart_isr & USART_ISR_TC) && (ubx.state != UBX_Tx_Sync)) {
        ubxdrv_rx();
        //ubxevt_txdone(0);
    }
    
    else {
        sub_ubx_close();
    }
}


void __DMARX_ISR(void) {
    ubxdrv_isr();
}

void __DMATX_ISR(void) {
    ubxdrv_isr();
}








/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */
ot_u8 ubxdrv_footerbytes() {
    return UBX_FOOTERBYTES;
}


ot_int ubxdrv_init(void* port_id) {
/// 1. "port_id" is unused in this impl, and it may be NULL
/// 2. Prepare the HW, which in this case is a UART
/// 3. Set default speed, which is the power-on speed of the module: 9600 bps

    /// Put reset pin low
    ubxdrv_reset();
    
    /// Basic setup for UBX-UART, with preset bps and sync detection
    ubx.state     = UBX_Null;
    ubx.baudrate  = UBXBAUD_STARTUP;

    /// UART Setup (RX & TX setup takes place at time of startup)
    __UART_CLKON();
    GNSS_UART->BRR = __SET_USARTDIV(UBXBAUD_STARTUP);
    GNSS_UART->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
    GNSS_UART->CR2 = 0;
    GNSS_UART->CR1 = 0;
    __UART_CLKOFF();

    /// Set up DMA channels for RX and TX
    _DMARX->CPAR    = (uint32_t)&(GNSS_UART->RDR);
    _DMATX->CPAR    = (uint32_t)&(GNSS_UART->TDR);

    /// GNSS RX & TX DMA Interrupts
    NVIC_SetPriority(_DMARX_IRQ, _IRQGROUP);
    NVIC_EnableIRQ(_DMARX_IRQ);
    NVIC_SetPriority(_DMATX_IRQ, _IRQGROUP);
    NVIC_EnableIRQ(_DMATX_IRQ);

    /// GNSS RXNE/TXE USART Interrupt & RX Pin Interrupts are used with
    /// the "Break" mode of operation.  NVIC configuration of Line interrupts
    /// must be in core_main.c, due to the way EXTIs are shared on STM32.
    NVIC_SetPriority(_UART_IRQ, _IRQGROUP);
    NVIC_EnableIRQ(_UART_IRQ);
    
#   if (__USE_RXBREAK)
    EXTI->PR    = GNSS_UART_RXPIN;
    EXTI->RTSR |= GNSS_UART_RXPIN;
#   endif
    
    return 255;
}


void ubxdrv_reset(void) {
/// ubxdrv_reset() does a HW Reset of the GNSS Module.  This is a warm reset.
/// Some implementations use a cold reset instead of a warm reset.  For these
/// implementations, this function only ensures that GNSS device is OFF.
    
#if defined(BOARD_GNSS_RESETPIN) && defined(BOARD_GNSS_RESETPORT)
    // high-low-high on reset pin to ensure a clean reset pulse
    BOARD_GNSS_RESETPORT->BSRR = BOARD_GNSS_RESETPIN;
    delay_us(50);
    BOARD_GNSS_RESETPORT->BSRR = (BOARD_GNSS_RESETPIN << 16);
    delay_us(50);
    BOARD_GNSS_RESETPORT->BSRR = BOARD_GNSS_RESETPIN;

#elif defined(BOARD_GNSS_SDNPIN) && defined(BOARD_GNSS_SDNPORT)
    BOARD_GNSS_TURNOFF();
    
#else
#   warning "GNSS board configuration does not have Reset or Shutdown pins defined."
    
#endif
}


ot_long ubxdrv_turnon(void) {
///@note dwell time for turn-on set to 20 ticks (ms), but this is arbitrary.
///      only data we can find has hard reset operation taking 10ms.  It needs
///      to be long enough for capacitance / inductance in the powering-up to 
///      be overcome.  Experimentation will ensue.
    BOARD_GNSS_TURNON();
    return 20;
}


void ubxdrv_turnoff(void) {
/// Kill the RX connection and cut the power to the GNSS module.
/// The power cutoff feature is an inline function from the board support
/// header, so board may not support it, in which case the call does nothing.
/// Make sure to check your board documentation about this.
    ubxdrv_kill();
    BOARD_GNSS_TURNOFF();
}


ot_long ubxdrv_extwakeup(void) {
	BOARD_GNSS_INTON();
	return 20;
}

void ubxdrv_extsleep(void) {
	BOARD_GNSS_INTOFF();
	ubxdrv_kill();
}


void ubxdrv_setbaudrate(ot_u32 baud_rate) {
/// This will take down the communication while baud rate is reset.  It will
/// return communication to default passive RX after finishing.
    sub_ubx_close();
    __UART_CLKON();
    GNSS_UART->BRR = __SET_USARTDIV(baud_rate); 
    
    ubxdrv_rx();
}


ot_u8 ubxdrv_getpwrcode(void) {
/// Power code: 0-3.  For this GNSS impl it's always 1 or 2
    return 1 + (ubx.state < 0);
}

void ubxdrv_standby(void) {
}

void ubxdrv_detach(void* port_id) {
    ubx.state = UBX_Null;
}

void ubxdrv_clear(void) {
    __CLR_UBX();
}


///@todo consider getting rid of block/unblock
void ubxdrv_block(void) {
    ubx.state = UBX_Null;
}

void ubxdrv_unblock(void) {
    if (ubx.state == UBX_Null) {
        ubx.state = UBX_Idle;
    }
}

void ubxdrv_kill(void) {
    // Close down the UART peripheral
    sub_ubx_close();
}

void ubxdrv_wait(void) {
    ///@todo add some type of watchdog?
    while (ubx.state != UBX_Idle);
}





void sub_ubx_txcont(void) {
    _DMATX->CCR         = 0;
    _DMATX->CMAR        = (ot_u32)&ubx.buf.header;
    _DMATX->CNDTR       = (ot_u16)ubx.frame_len + UBX_DMAFLUFF;
    DMA1->IFCR          = (_DMARX_IFG | _DMATX_IFG);     
    DMA1_CSELR->CSELR   = (DMA1_CSELR->CSELR & ~_DMA_CSEL_MASK) | _DMATX_CSEL;
    __UART_CLEAR();
    _DMATX->CCR         = (DMA_CCR_DIR | DMA_CCR_MINC | (2<<DMA_CCR_PL_Pos) | DMA_CCR_TCIE | DMA_CCR_EN);
}


ot_u16 ubxdrv_txsync(void) {
/// Send a single FF byte and return half a second, per spec.
    sub_ubx_open();
    __UART_CLEAR();
    ubx.state       = UBX_Tx_Sync;
    GNSS_UART->CR1  = USART_CR1_UE;
    GNSS_UART->TDR  = 0xFF;
    GNSS_UART->CR1  = (USART_CR1_UE | USART_CR1_TE | USART_CR1_TCIE);
    
    return 512;
}


ot_int ubxdrv_tx(void) {
/// Data TX will always occur on call.  RX will be stopped.
    ot_u16 holdtime;
    ot_uni16 fletcher;

    // close ongoing UART process (if any)
    sub_ubx_close();
    
    // Set up TX
    __SYS_CLKON();

    ubx.frame_len   = ubx.buf.header.length + sizeof(ubx_header_t);
    ubx.cursor      = (ot_u8*)&ubx.buf.header + ubx.frame_len;
    fletcher.ushort = sub_ubx_fletcher((ot_u8*)&ubx.buf.header, ubx.frame_len);
    *ubx.cursor++   = fletcher.ubyte[0];
    *ubx.cursor++   = fletcher.ubyte[1];
    ubx.frame_len  += 2;
    holdtime        = __UBX_TIMEOUT(ubx.frame_len+2);
    
    sub_ubx_open();
    ubx.state = UBX_Tx_Wait;
    __UART_TXOPEN();
    
    ubx.buf.sync.syncB5 = 0xb5;
    ubx.buf.sync.sync62 = 0x62;
    __DMA_TXOPEN(&ubx.buf.sync.syncB5, 2);

    return holdtime;
}




void ubxdrv_rx(void) {
    sub_ubx_close();               // State will be UBX_NULL until RX edge detect
    
    // Buffer emptying process
    ubx.cursor = (ot_u8*)&ubx.buf.header;
    
#   if (__USE_RXBREAK)
    EXTI->IMR  |= GNSS_UART_RXPIN;
#   else
    ubxdrv_rxsync_isr();
#   endif
}






void ubxdrv_isr(void) {
/// GNSS is state-based.  Depending on the GNSS implementation and the HW
/// implementation of the DMA+UART, state transitions may happen differently.
/// <LI> In typical RX, there is a header detection event that sets-up a second
///      RX process for downloading the rest of the packet.  When the DMA is
///      done, the process completes.  </LI>
    //ot_bool tx_process = False;
    ot_int      error_code;
    ot_uni16    fletcher;

    __DMA_ALL_CLOSE();
    __DMA_ALL_CLEAR();

    switch (ubx.state) {
        case UBX_Idle: //note, case doesn't break!

        case UBX_RxHeader: {        
            // If there is no payload or if the input queue is being used by 
            // someone else, this packet is registered as an error.
            ubx.frame_len   = ubx.buf.header.length;
            ubx.cursor     += UBX_OVERHEADBYTES;
            
            // Packet is 0 length: hence it is done
            if (ubx.frame_len == 0) {
                goto ubxdrv_isr_RXSIG;
            }
            
            // Check to see if length is out of bounds.
            else if (ubx.frame_len > sizeof(ubx_buf_t)) {
                error_code = -1;
                goto ubxdrv_isr_EXIT;
            }
            
            // Length in bounds, receive rest of packet
            // Most important first thing is to reset DMA to grab first frame.
            else {
                __DMA_RXOPEN(ubx.cursor, ubx.frame_len);
                ubx.state = UBX_RxPayload;
            }
        } return;             // Wait for next DMA RX interrupt

        case UBX_RxPayload: {
            ubx.cursor += ubx.frame_len;
        } goto ubxdrv_isr_RXSIG;

        case UBX_Tx_Wait:
            ubx.state = UBX_Tx_Done;
            sub_ubx_txcont();
            return;
        
        case UBX_Tx_Done:
            __UART_CLEAR();
            GNSS_UART->CR1 = (USART_CR1_UE | USART_CR1_TE | USART_CR1_TCIE);
            return; 
       
       default: break;
    }

    // The RX process is complete
    // - Close UBX and call rxdone event handler from UBX Task
    ubxdrv_isr_RXSIG:
    fletcher.ubyte[1]   = *(--ubx.cursor);
    fletcher.ubyte[0]   = *(--ubx.cursor);
    error_code          = fletcher.ushort - sub_ubx_fletcher((ot_u8*)&ubx.buf.header, ubx.cursor - (ot_u8*)&ubx.buf.header);
    
    // This driver runs pretty tight -- the amount of time to process the 
    // packet in ubxevt_rxdone is the amount of time it takes to receive the 
    // header of the next packet.
    ubxdrv_isr_EXIT:
    ubxdrv_rx();
    ubxevt_rxdone(error_code);
}







#endif

