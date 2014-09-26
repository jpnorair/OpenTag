/* Copyright 2010-2014 JP Norair
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
  * @file       /platform/stm32l1xx/otsys_mpipe_uart.c
  * @author     JP Norair
  * @version    R103
  * @date       21 Sept 2014
  * @brief      Message Pipe v2 (MPIPEv2) UART implementation for STM32L0xx
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
  * Flow control:           None, DTR, RTS/CTS
  * Connection:             RS-232, DTE-DTE (use a null-modem connector)
  *
  * Design Assumptions:
  * <LI> The peripheral bus clock is of high-enough frequency to cleanly
  *         generate 115200 baud -- 1.8432 MHz or higher is recommended.  </LI>
  *
  * MPipe Protocol for Serial:
  * <PRE>
  * +-------+-----------+-------+-----------+----------+---------+---------+
  * | Field | Sync Word | CRC16 | P. Length | Sequence | Control | Payload |
  * | Bytes |     2     |   2   |     2     |     1    |    1    |    N    |
  * | Value |   FF55    |       |     N     |   0-255  |   RFU   |   ALP   |
  * +-------+-----------+-------+-----------+----------+---------+---------+
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
  *
  * The MPipe driver also supports a Raw-ALP build.  MPIPE_USE_MAC can be set
  * to zero through the board config header or in this file (Default is ON).
  * Raw ALP does not include NDEF-compliant header or any footer.  Therefore,
  * there is no ACK ability.  Raw ALP has a 4-byte ALP header, identical to
  * NDEF header except Type Length and ID Length are removed.  Type Length is
  * implicitly 0 and ID Length is implicitly 2.
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>

#ifndef BOARD_PARAM_MPIPE_IFS
#   define BOARD_PARAM_MPIPE_IFS 1
#endif

#define MPIPEDRV_ENABLED        (BOARD_FEATURE(MPIPE))
#define THIS_MPIPEDRV_SUPPORTED ((BOARD_PARAM(MPIPE_IFS) == 1) && defined(MPIPE_UART))

#if (defined(__STM32L1__) && MPIPEDRV_ENABLED && THIS_MPIPEDRV_SUPPORTED)

#include <otlib/buffers.h>
#include <otlib/crc16.h>
#include <otsys/mpipe.h>
#include <otsys/sysclock.h>


/** MPipe Feature Configuration for UART, considering other modes <BR>
  * ========================================================================<BR>
  * There are multiple MPIPE modes.  Some platforms/boards support multiplexing
  * the MPIPE across different peripherals and with different options that are
  * made available at runtime.
  */
#if ((BOARD_FEATURE(MPIPE) == ENABLED) && (OT_FEATURE(MPIPE) == DISABLED))
#   warning "MPIPE is enabled in your board config but not your app config.  Usually this doesn't work."
#endif
#if (BOARD_FEATURE(MPIPE_FLOWCTL) || BOARD_FEATURE(MPIPE_CS))
#   error "This driver does not presently support CTS/RTS or DTR."
#endif
#if (BOARD_FEATURE(MPIPE_DIRECT) && BOARD_FEATURE(MPIPE_BREAK))
#   error "This driver presently supports Direct EITHER/OR Break modes, not both at the same time."
#endif

#define MPIPE_HEADERBYTES    8
#define MPIPE_FOOTERBYTES    0

#define MPIPE_OVERHEADBYTES (MPIPE_HEADERBYTES + MPIPE_FOOTERBYTES)
//#define MPIPE_UARTMODES     ( (BOARD_FEATURE_MPIPE_DIRECT == ENABLED) \
                            + (BOARD_FEATURE_MPIPE_CS == ENABLED)     \
                            + (BOARD_FEATURE_MPIPE_FLOWCTL == ENABLED))
#define MPIPE_UARTMODES     1
#define MPIPE_MODES         (MPIPE_UARTMODES)




/** MPIPE Local Buffer Setup  <BR>
  * ========================================================================<BR>
  * Without a local buffer, there is a time period after a packet is being
  * received (when it is being processed) where a new packet will be ignored.
  * With the local buffer, a packet coming during this time will get paged in
  * buffer, thus offering true non-blocking behavior.
  *
  * 64 bytes is usually sufficient, because at 500kbaud it takes ~1.3 ticks for
  * the buffer to fill-up.  The buffer should take at least 1 tick to fill.  If
  * you want the nitty-gritty details, read-on, otherwise just set to 64 bytes
  * and forget about it.
  *
  * Extra notes on buffer size:
  * The most safe way to configure buffer size is to make it big enough that
  * the fill-time of the buffer, when using fastest supported baudrate, is
  * greater than the worst-case time to process an inbound packet.  The OpenTag
  * MPipe task offers additional safety by queuing packets for processing into
  * its own application queue (typically this is the global stream "otmpin"),
  * but if otmpin gets filled-up, further inbound packets will be dropped until
  * otmpin empties-out enough to hold the next inbound packet.  In practical
  * usage, this will never happen, but it is something to think about if you
  * are doing cool new hacks.
  */
#define MPIPE_BUFFER_SIZE    64

#define MPIPE_CTL_NOCRC     1



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
#if MCU_CONFIG(MULTISPEED)
#   if BOARD_FEATURE(FULLSPEED)
#       define __SYS_CLKON()    (uart.clkhandle = sysclock_request(SPEED_Full))
#       define __SYS_CLKOFF()   sysclock_dismiss(uart.clkhandle)
#   elif BOARD_FEATURE(FLANKSPEED)
#       define __SYS_CLKON()    (uart.clkhandle = sysclock_request(SPEED_Flank))
#       define __SYS_CLKOFF()   sysclock_dismiss(uart.clkhandle)
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



/** MPIPE Peripheral Mapping  <BR>
  * ========================================================================<BR>
  * The STM32L1 can support the following channels for UART DMA:
  * UART1 TX/RX:    Ch4 / Ch5
  * UART2 TX/RX:    Ch7 / Ch6
  * UART3 TX/RX:    Ch2 / Ch3 
  *
  */
#if (MPIPE_UART_ID == 1)
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
#   define __UART_CLKHZ()   platform_get_clockhz(2)
#   define __UART_CLKON()   (RCC->APB2ENR |= RCC_APB2ENR_USART1EN)
#   define __UART_CLKOFF()  (RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN)

#elif (MPIPE_UART_ID == 2)
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
#   define __UART_CLKHZ()   platform_get_clockhz(1)
#   define __UART_CLKON()   (RCC->APB1ENR |= RCC_APB1ENR_USART2EN)
#   define __UART_CLKOFF()  (RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN)

#elif (MPIPE_UART_ID == 3)
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
#   define __UART_CLKHZ()   platform_get_clockhz(1)
#   define __UART_CLKON()   (RCC->APB1ENR |= RCC_APB1ENR_USART3EN)
#   define __UART_CLKOFF()  (RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN)

#else
#   error "MPIPE_UART_ID is misdefined, must be 1, 2, or 3."

#endif





/** Peripheral Control Macros  <BR>
  * ========================================================================<BR>
  */
#if (BOARD_FEATURE(MPIPE_DIRECT))
#   define __UART_TXOPEN() do { \
            MPIPE_UART->CR1 = 0; \
            MPIPE_UART->SR  = 0; \
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_TE); \
        } while (0)

#   define __UART_RXOPEN() do { \
            MPIPE_UART->CR1 = 0; \
            MPIPE_UART->SR  = 0; \
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_RE);   \
        } while (0)

#   define __UART_CLOSE()   (MPIPE_UART->CR1 = 0)
#   define __UART_CLEAR()   (MPIPE_UART->SR  = 0)
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Idle)

#elif (BOARD_FEATURE(MPIPE_BREAK))
#   define __UART_TXOPEN() do { \
            MPIPE_UART->CR1 = 0; \
            MPIPE_UART->SR  = 0; \
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_TE); \
        } while (0)

#   define __UART_RXOPEN() do { \
            MPIPE_UART->CR1 = 0; \
            MPIPE_UART->SR  = 0; \
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE);   \
        } while (0)

#   define __UART_CLOSE()   (MPIPE_UART->CR1 = 0)
#   define __UART_CLEAR()   (MPIPE_UART->SR  = 0)
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Null)
#endif


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





/** MPipe Macros
  * ========================================================================<BR>
  */

/// @todo lookup actual data rate in use: this is hardcoded for 115200
#define __MPIPE_TIMEOUT(BYTES)  (1 + ((_miti_per_byte[uart.baudrate] * BYTES) >> 10))




/** Mpipe Driver Data  <BR>
  * ========================================================================<BR>
  */
static const ot_u16 _miti_per_byte[ (MPIPE_500000bps + 1) ] = { \
    (1093),     // 9600
    (377),      // 28800
    (193),      // 57600
    (92),       // 115200
    (42),       // 250000
    (21)        // 500000
};

static const ot_u32 _brtable[ (MPIPE_500000bps + 1) ] = { \
    MCU_PARAM(UART_9600BPS),
    MCU_PARAM(UART_28800BPS),
    MCU_PARAM(UART_57600BPS),
    MCU_PARAM(UART_115200BPS),
    MCU_PARAM(UART_250000BPS),
    MCU_PARAM(UART_500000BPS)
};



typedef struct {
    ot_u8   syncFF;
    ot_u8   sync55;
    ot_u16  crc16;
    ot_u16  plen;
    ot_u8   seq;
    ot_u8   ctl;
} mpipe_header;


typedef struct {
#if (MPIPE_USE_ACKS)
    mpipe_priority  priority;
#endif
#if (MCU_CONFIG(MULTISPEED))
    ot_int          clkhandle;
#endif
    mpipe_speed     baudrate;
    ot_int          packets;
    ot_u8*          pkt;
    crcstream_t     crc;
    ot_queue        lq;
    ot_u8           rxbuffer[MPIPE_BUFFER_SIZE];
    mpipe_header    header;

} uart_struct;

uart_struct uart;








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
void sub_txopen();
void sub_txcont();
void sub_txstart(ot_bool blocking, mpipe_priority data_priority);
void sub_rx(ot_bool blocking, mpipe_priority data_priority);



void sub_mpipe_open() {
    ot_u32 scratch;
    __SYS_CLKON();
    __UART_CLKON();

    scratch                 = BOARD_UART_PORT->MODER;
    scratch                &= ~((3 << (BOARD_UART_TXPINNUM*2)) | (3 << (BOARD_UART_RXPINNUM*2)));
    scratch                |= (2 << (BOARD_UART_TXPINNUM*2)) | (2 << (BOARD_UART_RXPINNUM*2));
    BOARD_UART_PORT->MODER  = scratch;
    mpipe.state             = MPIPE_Idle;
}

void sub_mpipe_close() {
    ot_u32 scratch          = BOARD_UART_PORT->MODER;
    scratch                &= ~((3 << (BOARD_UART_TXPINNUM*2)) | (3 << (BOARD_UART_RXPINNUM*2)));
    scratch                |= (1 << (BOARD_UART_TXPINNUM*2)) | (0 << (BOARD_UART_RXPINNUM*2));
    BOARD_UART_PORT->MODER  = scratch;

    __DMA_ALL_CLOSE();
    __DMA_ALL_CLEAR();
    __DMA_CLKOFF();
    __UART_CLOSE();
    __UART_CLEAR();
    __UART_CLKOFF();

    //__DISMISS_FULL_SPEED();
    __CLR_MPIPE();
}


void mpipe_rxsync_isr(void) {
/// Falling edge ISR on RX line, which can wakeup from stop
    EXTI->IMR  &= ~MPIPE_UART_RXPIN;
    sub_mpipe_open();
    __UART_RXOPEN();
}


void __UART_ISR(void) {
//    if (MPIPE_UART->SR & USART_SR_RXNE) {
        uart.rxbuffer[0] = MPIPE_UART->DR;
        mpipe.state     += (uart.rxbuffer[0] == 0x55);

        if (mpipe.state > MPIPE_Idle) {
            //mpipeevt_rxsync(0);
            MPIPE_UART->CR1   = (USART_CR1_UE | USART_CR1_RE);
            __DMA_RXOPEN((ot_u8*)&uart.header.plen, MPIPE_HEADERBYTES-2);
        }
//    }
//    else if (MPIPE_UART->SR & USART_SR_TXE) {
//        if (--uart.trailer == 0) {
//            mpipedrv_isr_TXSIG:
//            mpipedrv_kill();
//            mpipeevt_txdone(0);
//        }
//    }
}


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








/** Mpipe Main Public Functions  <BR>
  * ========================================================================
  */
#ifndef EXTF_mpipedrv_footerbytes
ot_u8 mpipedrv_footerbytes() {
    return MPIPE_FOOTERBYTES;
}
#endif


#ifndef EXTF_mpipedrv_init
ot_int mpipedrv_init(void* port_id, mpipe_speed baud_rate) {
/// 1. "port_id" is unused in this impl, and it may be NULL
/// 2. Prepare the HW, which in this case is a UART
/// 3. Set default speed, which in this case is 115200 bps
    ot_u8 baud_id;

    /// Basic setup for MPIPE-UART, with 57600bps and sync detection
    mpipe.state     = MPIPE_Null;
    uart.baudrate   = baud_rate;

    /// UART Setup (RX & TX setup takes place at time of startup)
    __UART_CLKON();
    MPIPE_UART->BRR = __UART_CLKHZ() / _brtable[uart.baudrate];
    MPIPE_UART->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
    MPIPE_UART->CR2 = 0;
    MPIPE_UART->CR1 = 0;
    __UART_CLKOFF();

    /// Set up DMA channels for RX and TX
    _DMARX->CPAR    = (uint32_t)&(MPIPE_UART->DR);
    _DMATX->CPAR    = (uint32_t)&(MPIPE_UART->DR);

    /// MPipe RX & TX DMA Interrupts
    NVIC->IP[(ot_u32)_DMARX_IRQ]        = _IRQGROUP;
    NVIC->ISER[(ot_u32)(_DMARX_IRQ>>5)] = (1 << ((ot_u32)_DMARX_IRQ & 0x1F));
    NVIC->IP[(ot_u32)_DMATX_IRQ]        = _IRQGROUP;
    NVIC->ISER[(ot_u32)(_DMATX_IRQ>>5)] = (1 << ((ot_u32)_DMATX_IRQ & 0x1F));

    /// MPipe RXNE/TXE USART Interrupt & RX Pin Interrupts are used with
    /// the "Break" mode of operation.  NVIC configuration of Line interrupts
    /// must be in core_main.c, due to the way EXTIs are shared on STM32.
#   if (BOARD_FEATURE(MPIPE_BREAK))
        NVIC->IP[(ot_u32)_UART_IRQ]         = _IRQGROUP;
        NVIC->ISER[(ot_u32)(_UART_IRQ>>5)]  = (1 << ((ot_u32)_UART_IRQ & 0x1F));
        EXTI->PR                            = MPIPE_UART_RXPIN;
        EXTI->RTSR                         |= MPIPE_UART_RXPIN;
#   endif


    ///@todo this will need to be adjusted in the final version
    alp_init(&mpipe.alp, &otmpin, &otmpout);

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
    ///@note I just commented-out this on 20 Sept
	//q_empty(mpipe.alp.outq);
}
#endif



#ifndef EXTF_mpipedrv_wait
void mpipedrv_wait() {
    while (mpipe.state != MPIPE_Idle);
}
#endif



#ifndef EXTF_mpipedrv_tx
void sub_txopen() {
    // set state to TX wait, which will come after header is TX'ed
    mpipe.state         = MPIPE_Tx_Wait;
    uart.header.syncFF  = 0xff;
    uart.header.sync55  = 0x55;
    uart.header.plen    = PLATFORM_ENDIAN16( q_span(&uart.lq) );
    uart.header.ctl     = 0;
    uart.header.seq    += 1;
    
    //CTL in this is always enabling CRC (see above)
    //if ((uart.header.ctl & MPIPE_CTL_NOCRC) == 0) {
        uart.header.crc16 = crc16drv_block_manual((ot_u8*)&uart.header.plen, 4, 0xFFFF);
        uart.header.crc16 = crc16drv_block_manual(  uart.lq.getcursor,
                                                    uart.header.plen,
                                                    uart.header.crc16   );
        uart.header.crc16 = PLATFORM_ENDIAN16(uart.header.crc16);
    //}

    sub_mpipe_close();
    sub_mpipe_open();
    __UART_TXOPEN();
    __DMA_TXOPEN(&uart.header.syncFF, 8);
}

OT_INLINE_H void sub_txcont() {
    __DMA_TXOPEN(uart.lq.getcursor, q_span(&uart.lq));
}

ot_int mpipedrv_tx(ot_bool blocking, mpipe_priority data_priority) {
/// Data TX will only occur if this function is called when the MPipe state is
/// idle.  The exception is when the function is called with ACK priority, in
/// which case the state doesn't need to be Idle.  Lastly, if you specify the
/// blocking parameter, the function will not return until the packet is
/// completely transmitted.
    ot_u16 holdtime;
    ot_u16 pktlen;
    
#   if (MPIPE_USE_ACKS)
//    if (data_priority == MPIPE_Ack)) {
//        uart.priority  = data_priority;
//        ///@todo In this space, the ACK needs to be swapped to the queue
//        goto mpipedrv_tx_GO;
//    }
//    ///@todo In this space, swap to the standard mpipe queue
#   endif

    holdtime = q_blocktime(mpipe.alp.outq);
    if (holdtime != 0) {
        return -holdtime;
    }
    
    //getcursor to end of packet, to allow another packet to be added
    uart.lq.front               = mpipe.alp.outq->getcursor;
    mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;
    uart.lq.back                = mpipe.alp.outq->putcursor;
    pktlen                      = q_length(&uart.lq);
    holdtime                    = __MPIPE_TIMEOUT(pktlen);

    if (mpipe.state == MPIPE_Idle) {
        uart.lq.getcursor   = uart.lq.front;
        uart.lq.putcursor   = uart.lq.front + pktlen;
        mpipedrv_tx_GO:
        __SYS_CLKON();
        sub_txopen();
        q_blockwrite(mpipe.alp.outq, blocking ? holdtime : 0);
    }

    return __MPIPE_TIMEOUT( holdtime );
}
#endif




ot_u16 mpipedrv_txsync() {
/// Send a single FF byte and return the timeout (We have it at 6).
/// We don't care about interrupts, because it is just one byte.
    sub_mpipe_open();
    __UART_TXOPEN();
    MPIPE_UART->DR = 0xFF;
    return 6;
}






/*
#ifndef EXTF_mpipedrv_rx
void sub_rx(ot_bool blocking, mpipe_priority data_priority) {
#if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack) {
        uart.priority  = data_priority;
        goto mpipedrv_rx_SETUP;
    }
#endif
    if (blocking) {
        mpipedrv_wait();
    }
    if (mpipe.state == MPIPE_Idle) {
        mpipedrv_rx_SETUP:
        //mpipe.state = MPIPE_RxHeader;
        q_empty(mpipe.alp.inq);
        //mpipe.alp.inq->back -=10;

        __SYS_CLKON();
        __UART_CLKON();
        __DMA_CLKON();
        __UART_RXOPEN();
        __DMA_RXOPEN(mpipe.alp.inq->front, MPIPE_OVERHEADBYTES);
    }
}

void mpipedrv_rx(ot_bool blocking, mpipe_priority data_priority) {
    /// DIRECT UART Only
    sub_rx(blocking, data_priority);
}
#endif
*/




#ifndef EXTF_mpipedrv_rx
void mpipedrv_rx(ot_bool blocking, mpipe_priority data_priority) {
#if (BOARD_FEATURE(MPIPE_BREAK))
/// Wait for Sync.  Sync interrupt will call sub_rx()
    sub_mpipe_close();
    EXTI->IMR  |= MPIPE_UART_RXPIN;

#elif (BOARD_FEATURE(MPIPE_DIRECT))
    sub_mpipe_open();
    __UART_RXOPEN();

    mpipe.state       = MPIPE_RxPayload;
    MPIPE_UART->CR1   = (USART_CR1_UE | USART_CR1_RE);
    __DMA_RXOPEN(&uart.rxbuffer[0], uart.rxbuffer[0]+2);
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
    ot_int  error_code;

    __DMA_ALL_CLOSE();
    __DMA_ALL_CLEAR();

    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!

        case MPIPE_RxHeader: {
            // Use local queue, in order to protect the pending data from
            // getting accessed by other users of the main queue.
            q_copy(&uart.lq, mpipe.alp.inq);
        
            // If there is no payload or if the input queue is being used by 
            // someone else, this packet is registered as an error.
            uart.header.plen = PLATFORM_ENDIAN16(uart.header.plen);
            if (uart.header.plen == 0)                          error_code = -1;
            else if (q_blocktime(mpipe.alp.inq))                error_code = -11;
            else if (q_space(mpipe.alp.inq) < uart.header.plen) error_code = -7;
            
            // No error, start receiving packet formally
            // Most important first thing is to reset DMA to grab first frame.
            else {
                ot_u16 blockticks;
                ot_u16 framesize;
                
                blockticks = __MPIPE_TIMEOUT(uart.header.plen);
                q_blockwrite(mpipe.alp.inq, blockticks);
                
                framesize = (uart.header.plen <= MPIPE_BUFFER_SIZE) ? uart.header.plen : MPIPE_BUFFER_SIZE;
                __DMA_RXOPEN(&uart.rxbuffer, framesize);
                mpipeevt_rxdetect(blockticks);
                mpipe.state = MPIPE_RxPayload;
                
                // Start-up the CRC streamer manually.
                // uart.crc.count is also used to track the bytes left to receive
                uart.crc.count  = uart.header.plen;
                uart.crc.val    = 0;
                if ((uart.header.ctl & MPIPE_CTL_NOCRC) == 0) {
                    uart.crc.writeout   = False;
                    uart.crc.cursor     = (ot_u8*)&uart.header.crc16;
                    uart.crc.count     += 6;
                    crc_calc_nstream(&uart.crc, 6);
                    uart.crc.cursor     = uart.lq.putcursor;
                }
                return;             // Wait for next DMA RX interrupt
            }
        } goto mpipedrv_isr_RXSIG;  // handle error

        case MPIPE_RxPayload: {
            ot_u16  lastframe   = _DMARX->CNDTR;    // Last transferred amount of bytes
            ot_u16  nextframe;
            
            // If there are more frames after this one, refresh DMA buffering
            if (uart.crc.count > MPIPE_BUFFER_SIZE) {
                nextframe = uart.crc.count - MPIPE_BUFFER_SIZE;
                if (nextframe > MPIPE_BUFFER_SIZE) {
                    nextframe = MPIPE_BUFFER_SIZE;
                }
                __DMA_RXOPEN(&uart.rxbuffer, nextframe);
            }
            
            // Write the last frame data received to the app/alp queue.
            q_writestring(&uart.lq, uart.rxbuffer, lastframe);
            
            // Do CRC if required, else manually adjust crc.count
            if ((uart.header.ctl & MPIPE_CTL_NOCRC) == 0) {
                crc_calc_nstream(&uart.crc, lastframe);
            }
            else {
                uart.crc.count -= lastframe;
            }
            
            // Update the hold time on the main queue.  This is not required,
            // and it is a bit of a hack, but it is nice to do
            q_blockwrite(mpipe.alp.inq, __MPIPE_TIMEOUT(uart.crc.count));

            // Packet is done being received: check CRC and, if valid, refresh
            // the main alp queue.
            if (uart.crc.count <= 0) {
                if (uart.crc.val == uart.header.crc16) {
                    q_copy(mpipe.alp.inq, &uart.lq);
                    error_code = 0;   
                }
                else {
                    error_code = -2;
                }         
#               if (MPIPE_USE_ACKS)
                // ACKs must be used when Broadcast mode is off
                // 1. On ACKs, tx() requires caller to choose state
                // 2. Copy RX'ed seq number into local seq number
                // 3. Copy NACK/ACK status to 6th byte in NDEF header
                if (uart.priority != MPIPE_Broadcast) {
                    mpipe.state = MPIPE_TxAck_Done; //MPIPE_TxAck_Wait;
                    sub_txack_header(error_code);
                    mpipedrv_tx(False, MPIPE_Ack);
                    return;
                }
#               endif    
            }
        } goto mpipedrv_isr_RXSIG;

#       if (MPIPE_USE_ACKS)
//      case MPIPE_TxAck_Wait:
//            //MPIPE_UART->IE = UCTXIE;
//            //return;
//
//        case MPIPE_TxAck_Done:  // TX'ed an ACK
//            if (mpipe.alp.outq->front[3] != 0) { // TX'ed a NACK
//                mpipedrv_rx(False, uart.priority);
//                mpipe.state = MPIPE_RxHeader;
//                return;
//            }
//            uart.priority = MPIPE_Low;
//            goto mpipedrv_isr_RXSIG;
#       endif

        case MPIPE_Tx_Wait:
            mpipe.state = MPIPE_Tx_Done;
            sub_txcont();
            return;

#       if (MPIPE_USE_ACKS)
//        case MPIPE_Tx_Done:
//            if (uart.priority != MPIPE_Broadcast) {
//                mpipedrv_rx(False, MPIPE_Ack);
//                mpipe.state = MPIPE_RxAck;
//                return;
//            }
//            goto mpipedrv_isr_TXSIG;
#       endif

#       if (MPIPE_USE_ACKS)
//        case MPIPE_RxAck:
//            if (crc16drv_block(uart.rxbuffer, 8) != 0) { //RX'ed NACK
//                mpipedrv_tx(False, uart.priority);
//                return;
//            }
//            goto mpipedrv_isr_TXSIG;
#       endif

       //default: goto mpipedrv_isr_TXSIG;  //normal behavior is to fall through
    }

    // The TX process is complete, including any ACKing.
    // - Check the TX queue to see if there is anything queued.
    // - If yes, then piggyback the transmission
    // - If no, then close Mpipe and call txdone event handler in the MPipe Task
    mpipedrv_isr_TXSIG:
    while ((BOARD_UART_PORT->IDR & BOARD_UART_TXPIN) == 0);
    if (uart.lq.putcursor < uart.lq.back) {
        uart.lq.getcursor = uart.lq.putcursor;
        uart.lq.putcursor = uart.lq.back;
        sub_txopen();
        return;
    }
    mpipedrv_rx(False, 0);
    mpipeevt_txdone(0);
    return;

    // The RX process is complete, including and ACKing.
    // - Close MPipe and call rxdone event handler from MPipe Task
    // - If RX CRC matters, then make sure to compute it.
    mpipedrv_isr_RXSIG:
    mpipedrv_rx(False, 0);
    mpipeevt_rxdone(error_code);
}

#endif








#endif

