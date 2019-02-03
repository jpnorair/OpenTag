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
  * @file       /platform/stm32l0xx/otsys_mpipe_uart.c
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
#define THIS_MPIPEDRV_SUPPORTED ((BOARD_PARAM_MPIPE_IFS == 1) && defined(MPIPE_UART))

#if (defined(__STM32L0__) && OT_FEATURE_MPIPE && MPIPEDRV_ENABLED && THIS_MPIPEDRV_SUPPORTED)

#include <otlib/buffers.h>
#include <otlib/crc16.h>
#include <otsys/mpipe.h>
#include <otsys/sysclock.h>
#include <otlib/memcpy.h>


/** MPipe Default speed configuration <BR>
  * ========================================================================<BR>
  * Default baud is 115200, or whatever is put into the Board params.
  */
#ifndef BOARD_PARAM_MPIPEBAUD
#   define BOARD_PARAM_MPIPEBAUD 115200
#endif
#if (BOARD_PARAM_MPIPEBAUD <= 9600)
#   define _DEF_MPIPEBAUD   MPIPE_9600bps
#elif (BOARD_PARAM_MPIPEBAUD <= 28800)
#   define _DEF_MPIPEBAUD   MPIPE_28800bps
#elif (BOARD_PARAM_MPIPEBAUD <= 57600)
#   define _DEF_MPIPEBAUD   MPIPE_57600bps
#elif (BOARD_PARAM_MPIPEBAUD <= 115200)
#   define _DEF_MPIPEBAUD   MPIPE_115200bps
#elif (BOARD_PARAM_MPIPEBAUD <= 250000)
#   define _DEF_MPIPEBAUD   MPIPE_250000bps
#elif (BOARD_PARAM_MPIPEBAUD <= 500000)
#   define _DEF_MPIPEBAUD   MPIPE_500000bps
#else
#   define _DEF_MPIPEBAUD   MPIPE_500000bps
#endif


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
#if defined(MPIPE_USE_ACKS)
#   warning "MPipe ACKing is disabled in this driver and unnecessary if you have a UART-USB."
#   undef MPIPE_USE_ACKS
#endif

#define MPIPE_HEADERBYTES   8
#define MPIPE_FOOTERBYTES   0
#define MPIPE_DMAFLUFF      0

#define MPIPE_OVERHEADBYTES (MPIPE_HEADERBYTES + MPIPE_FOOTERBYTES)
//#define MPIPE_UARTMODES   ((BOARD_FEATURE_MPIPE_DIRECT==ENABLED) + (BOARD_FEATURE_MPIPE_CS==ENABLED) + (BOARD_FEATURE_MPIPE_FLOWCTL==ENABLED))
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
#define MPIPE_BUFFER_SIZE   64



/** CRC Override  <BR>
  * ========================================================================<BR>
  * MPipe has a CRC16 feature, although it's superfluous in a system where
  * USB is moving data off-board (USB has its own CRCs).  You can disable it
  * by setting MPIPE_CTL_NOCRC to 1.  Transmissions coming from the device
  * will always have CRC, though, just in receptions it will be ignored.
  */
#define MPIPE_CTL_NOCRC     1



/** MPIPE Interrupt Configuration  <BR>
  * ========================================================================<BR>
  * NVIC parameters are negotiated or defaulted, considering the preset config
  * in the platform section.
  */
#ifndef BUILD_NVIC_SUBGROUP_MPIPE
#   define BUILD_NVIC_SUBGROUP_MPIPE 0
#endif

//#define _SUBGROUP   (0)
#define _IRQGROUP   (PLATFORM_NVIC_IO_GROUP)

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
#if (MPIPE_UART_ID == 1)
#   if (MPIPE_DMA_RXCHAN_ID == 5)
#       define _DMARX       DMA1_Channel5
#       define __DMARX_ISR  platform_isr_dma1ch5
#   elif (MPIPE_DMA_RXCHAN_ID == 3)
#       define _DMARX       DMA1_Channel3
#       define __DMARX_ISR  platform_isr_dma1ch3
#   else
#       error "MPipe RX DMA not configured on channel 3 or 5."
#   endif
#   if (MPIPE_DMA_TXCHAN_ID == 4)
#       define _DMATX       DMA1_Channel4
#       define __DMATX_ISR  platform_isr_dma1ch4
#   elif (MPIPE_DMA_TXCHAN_ID == 2)
#       define _DMATX       DMA1_Channel2
#       define __DMATX_ISR  platform_isr_dma1ch2
#   else
#       error "MPipe TX DMA not configured on channel 2 or 4."
#   endif
#   define _UART_IRQ        USART1_IRQn
#   define _DMARX_IRQ       DMA1_Channel4_5_6_7_IRQn
#   define _DMATX_IRQ       DMA1_Channel4_5_6_7_IRQn
#   define _DMARX_IFG       (0xF << (4*(MPIPE_DMA_RXCHAN_ID-1)))
#   define _DMATX_IFG       (0xF << (4*(MPIPE_DMA_TXCHAN_ID-1)))
#   define _DMA_CSEL_MASK   ((0xF << (4*(MPIPE_DMA_RXCHAN_ID-1))) | (0xF << (4*(MPIPE_DMA_TXCHAN_ID-1))))
#   define _DMA_CSEL        ((0x3 << (4*(MPIPE_DMA_RXCHAN_ID-1))) | (0x3 << (4*(MPIPE_DMA_TXCHAN_ID-1))))
#   define _DMARX_CSEL      (0x3 << (4*(MPIPE_DMA_RXCHAN_ID-1)))
#   define _DMATX_CSEL      (0x3 << (4*(MPIPE_DMA_TXCHAN_ID-1)))
#   define __UART_ISR       platform_isr_usart1
#   define __UART_CLKHZ()   platform_get_clockhz(2)
#   define __UART_CLKON()   (RCC->APB2ENR |= RCC_APB2ENR_USART1EN)
#   define __UART_CLKOFF()  (RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN)

#elif (MPIPE_UART_ID == 2)
#   if (MPIPE_DMA_RXCHAN_ID == 5)
#       define _DMARX       DMA1_Channel5
#       define __DMARX_ISR  platform_isr_dma1ch5
#   elif (MPIPE_DMA_RXCHAN_ID == 6)
#       define _DMARX       DMA1_Channel6 
#       define __DMARX_ISR  platform_isr_dma1ch6
#   else
#       error "MPipe RX DMA not configured on channel 5 or 6."
#   endif
#   if (MPIPE_DMA_TXCHAN_ID == 4)
#       define _DMATX       DMA1_Channel4
#       define __DMATX_ISR  platform_isr_dma1ch4
#   elif (MPIPE_DMA_TXCHAN_ID == 7)
#       define _DMATX       DMA1_Channel7 
#       define __DMATX_ISR  platform_isr_dma1ch7
#   else
#       error "MPipe TX DMA not configured on channel 4 or 7."
#   endif
#   define _UART_IRQ        USART2_IRQn
#   define _DMARX_IRQ       DMA1_Channel4_5_6_7_IRQn
#   define _DMATX_IRQ       DMA1_Channel4_5_6_7_IRQn
#   define _DMARX_IFG       (0xF << (4*(MPIPE_DMA_RXCHAN_ID-1)))
#   define _DMATX_IFG       (0xF << (4*(MPIPE_DMA_TXCHAN_ID-1)))
#   define _DMA_CSEL_MASK   ((0xF << (4*(MPIPE_DMA_RXCHAN_ID-1))) | (0xF << (4*(MPIPE_DMA_TXCHAN_ID-1))))
#   define _DMA_CSEL        ((0x4 << (4*(MPIPE_DMA_RXCHAN_ID-1))) | (0x4 << (4*(MPIPE_DMA_TXCHAN_ID-1))))
#   define _DMARX_CSEL      (0x4 << (4*(MPIPE_DMA_RXCHAN_ID-1)))
#   define _DMATX_CSEL      (0x4 << (4*(MPIPE_DMA_TXCHAN_ID-1)))
#   define __UART_ISR       platform_isr_usart2
#   define __UART_CLKHZ()   platform_get_clockhz(1)
#   define __UART_CLKON()   (RCC->APB1ENR |= RCC_APB1ENR_USART2EN)
#   define __UART_CLKOFF()  (RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN)

//#elif (MPIPE_UART_ID == 3)
// This could be the LPUART, but that is not supported at the moment

#else
#   error "MPIPE_UART_ID is misdefined, must be 1, or 2"

#endif





/** Peripheral Control Macros  <BR>
  * ========================================================================<BR>
  */
#if (BOARD_FEATURE(MPIPE_DIRECT))
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Idle)

#elif (BOARD_FEATURE(MPIPE_BREAK))
#   define __CLR_MPIPE()    (mpipe.state = MPIPE_Null)

#else
#	error "MPIPE UART not set with MPIPE_DIRECT or MPIPE_BREAK operational parameter."
#endif

#define __UART_CLOSE()   (MPIPE_UART->CR1 = 0)
#define __UART_CLEAR()   (MPIPE_UART->ICR = (1<<20)|(1<<17)|(1<<12)|(1<<11)|(1<<9)|(1<<8)|(1<<6)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0))

#define __UART_TXOPEN() do { \
        MPIPE_UART->CR1 = 0; \
        __UART_CLEAR(); \
        MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_TE); \
    } while (0)

#define __UART_RXOPEN() do { \
        MPIPE_UART->CR1 = 0; \
        __UART_CLEAR(); \
        MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE);   \
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





/** MPipe Macros
  * ========================================================================<BR>
  */

/// @todo lookup actual data rate in use: this is hardcoded for 115200
//#define __MPIPE_TIMEOUT(BYTES)  (1 + ((_miti_per_byte[uart.baudrate] * BYTES) >> 10))
#define __MPIPE_TIMEOUT(BYTES)  ( 1 + ((BYTES+8) >> 3) )



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

typedef struct __attribute__((packed)) {
    ot_u8   syncFF;
    ot_u8   sync55;
    ot_u16  crc16;
    ot_u16  plen;
    ot_u8   seq;
    ot_u8   ctl;
} mpipe_header;


typedef struct {
    ot_u8*  front;
    ot_u16  length;
} uart_element_t;

#define UART_RB_MAX 8
typedef struct {
    uart_element_t  rb[UART_RB_MAX];
    ot_u8           reserved;
    ot_u8           size;
    ot_u8           i;
    ot_u8           j;
} uart_rb_t;

typedef struct {
#if (MPIPE_USE_ACKS)
    mpipe_priority  priority;
#endif
#if (MCU_CONFIG(MULTISPEED))
    ot_int          clkhandle;
#endif
    mpipe_speed     baudrate;
    crcstream_t     crc;
    mpipe_header    header;
    ot_u8           rxbuffer[MPIPE_BUFFER_SIZE];
    ot_u16          rxframelen;
    ot_u16          rxplen;
    uart_rb_t       tlist;
} uart_struct;

uart_struct uart;



#define _HEADER_RXPTR   	((ot_u8*)&uart.header.sync55)
#define _HEADER_RXOFFSET	1


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





/** Mpipe ISRs  <BR>
  * ========================================================================<BR>
  * There are two kinds of ISRs in this MPIPE driver: DMA ISRs and Pin ISRs.
  * The DMA ISRs occur when the data is finished being transmitted or received.
  * The Pin ISRs occur when there is a wake-up or handshaking signal detected.
  *
  * BREAK ISR: In break mode, there is a "break" character placed before the data.
  * It is used in the same way as DTR, except there is no additional line, just the
  * normal RX line that is attached to an edge interrupt.  it uses the normal RX
  * line instead of an additional CS/DTR line.  You can also safely use a 0x00
  * character instead of a break.
  *
  * These are DMA ISRs.  They are macros which are defined in this file, above.
  * In STM32, each DMA channel has its own IRQ, and each peripheral has mapping
  * to different channels.
  */

void mpipe_rxsync_isr(void) {
#   if (BOARD_FEATURE(MPIPE_BREAK))
    // Falling edge ISR on RX line, which can wakeup from stop
    EXTI->IMR  &= ~MPIPE_UART_RXPIN;
#   endif

    // Open UART for character-RX
    mpipe.state = MPIPE_Idle;
    // mpipeevt_rxinit();       ///@todo see if there's a point here
    sub_mpipe_open();
    __UART_RXOPEN();
}


void __UART_ISR(void) {
    ot_u32 uart_isr;
    uart_isr = MPIPE_UART->ISR;
    __UART_CLEAR();
    
    /// USART interrupts that we care about:
    /// - CM:   Character match, used to detect 0x55 sync byte.  Only for initial RX.
    /// - RXNE  RX register interrupt, used as alternate implementation of Sync Match
    /// - TC    b__: Transmission Complete interrupt
    
#   if 0
    // Experimental Character-Match
    if (uart_isr & USART_ISR_CMF) {
        mpipe.state         = MPIPE_RxHeader;
        uart.rxbuffer[0]    = MPIPE_UART->RDR;      // Clear RXNE flag
        MPIPE_UART->CR1     = (USART_CR1_UE | USART_CR1_RE);
        __DMA_RXOPEN(_HEADER_RXPTR, MPIPE_HEADERBYTES-_HEADER_RXOFFSET);
    }
#   else
    // Proven char-by-char detect
    if (uart_isr & USART_ISR_RXNE) {
        uart.rxbuffer[0] = MPIPE_UART->RDR;
        mpipe.state     += (uart.rxbuffer[0] == 0x55);  // Progress to MPIPE_RxHeader
        if (mpipe.state > MPIPE_Idle) {
            //mpipeevt_rxsync(0);
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_RE);
            __DMA_RXOPEN(_HEADER_RXPTR, MPIPE_HEADERBYTES-_HEADER_RXOFFSET);
        }
    }
    //else
#   endif

    // TX Complete
    else if (uart_isr & USART_ISR_TC) {
#       if (MPIPE_USE_ACKS)        
        if (uart.priority != MPIPE_Broadcast) {
            mpipedrv_rx(False, MPIPE_Ack);
            mpipe.state = MPIPE_RxAck;
        }
        else
#       endif
        if (--uart.tlist.size != 0) {
            uart.tlist.i = (uart.tlist.i + 1) & (UART_RB_MAX-1);
            sub_txopen();
        }
        else {
            mpipedrv_rx(False, 0);
            mpipeevt_txdone(0);
        }
    }    
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

    /// Basic setup for MPIPE-UART, with preset bps and sync detection
    mpipe.state     = MPIPE_Null;
    uart.baudrate   = (baud_rate == MPIPE_default) ? _DEF_MPIPEBAUD : baud_rate;

    /// UART Setup (RX & TX setup takes place at time of startup)
    __UART_CLKON();
    MPIPE_UART->BRR = __UART_CLKHZ() / _brtable[uart.baudrate];
    MPIPE_UART->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
    MPIPE_UART->CR2 = 0;
    MPIPE_UART->CR1 = 0;
    __UART_CLKOFF();

    /// Set up DMA channels for RX and TX
    _DMARX->CPAR    = (uint32_t)&(MPIPE_UART->RDR);
    _DMATX->CPAR    = (uint32_t)&(MPIPE_UART->TDR);

    /// MPipe RX & TX DMA Interrupts
    NVIC_SetPriority(_DMARX_IRQ, _IRQGROUP);
    NVIC_EnableIRQ(_DMARX_IRQ);
    NVIC_SetPriority(_DMATX_IRQ, _IRQGROUP);
    NVIC_EnableIRQ(_DMATX_IRQ);

    /// MPipe RXNE/TXE USART Interrupt & RX Pin Interrupts are used with
    /// the "Break" mode of operation.  NVIC configuration of Line interrupts
    /// must be in core_main.c, due to the way EXTIs are shared on STM32.
    NVIC_SetPriority(_UART_IRQ, _IRQGROUP);
    NVIC_EnableIRQ(_UART_IRQ);
    
#   if (BOARD_FEATURE(MPIPE_BREAK))
        EXTI->PR    = MPIPE_UART_RXPIN;
        EXTI->RTSR |= MPIPE_UART_RXPIN;
#   endif

    /// Configure MPipe Queues
    /// @todo this will need to be adjusted in the final version
    alp_init(&mpipe.alp, &otmpin, &otmpout);
    
    /// Initial values for Ring Buffer
    memset(&uart.tlist, 0, sizeof(uart_rb_t));
    
    return 255;
}
#endif


#ifndef EXTF_mpipedrv_getpwrcode
ot_u8 mpipedrv_getpwrcode() {
/// Power code: 0-3.  For this MPipe impl it's always 1 or 2
    return 1 + (mpipe.state < 0);
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
///@todo Check if this function gets used, and how.
void mpipedrv_unblock() {
    if (mpipe.state == MPIPE_Null) {
        mpipe.state = MPIPE_Idle;
    }
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
    ///@todo add some type of watchdog?
    while (mpipe.state != MPIPE_Idle);
}
#endif




void sub_txopen() {
    uart_element_t*  txpayload;
    
    txpayload           = &uart.tlist.rb[uart.tlist.i];

    /// Build Header
    uart.header.syncFF  = 0xff;
    uart.header.sync55  = 0x55;
    
    // Always include CRC on TX
    uart.header.plen    = PLATFORM_ENDIAN16(txpayload->length);
    uart.header.ctl     = 0;
    uart.header.seq    += 1;
    
    uart.header.crc16   = crc16drv_block_manual((ot_u8*)&uart.header.plen, 4, 0xFFFF);
    uart.header.crc16   = crc16drv_block_manual(txpayload->front, txpayload->length, uart.header.crc16);
    uart.header.crc16   = PLATFORM_ENDIAN16(uart.header.crc16);

    sub_mpipe_close();
    sub_mpipe_open();
    mpipe.state = MPIPE_Tx_Wait;
    __UART_TXOPEN();
    __DMA_TXOPEN(&uart.header.syncFF, 8);
}


void sub_txcont() {
    uart_element_t* txpayload;
    txpayload           = &uart.tlist.rb[uart.tlist.i];
    _DMATX->CCR         = 0;
    _DMATX->CMAR        = (uint32_t)txpayload->front;   
    _DMATX->CNDTR       = txpayload->length + MPIPE_DMAFLUFF;
    DMA1->IFCR          = (_DMARX_IFG | _DMATX_IFG);
    DMA1_CSELR->CSELR   = (DMA1_CSELR->CSELR & ~_DMA_CSEL_MASK) | _DMATX_CSEL;
    __UART_CLEAR();
    _DMATX->CCR         = (DMA_CCR_DIR | DMA_CCR_MINC | (2<<DMA_CCR_PL_Pos) | DMA_CCR_TCIE | DMA_CCR_EN);
}



#ifndef EXTF_mpipedrv_tx
ot_int mpipedrv_tx(ot_bool blocking, mpipe_priority data_priority) {
/// Data TX will only occur if this function is called when the MPipe state is
/// idle.  The exception is when the function is called with ACK priority, in
/// which case the state doesn't need to be Idle.  Lastly, if you specify the
/// blocking parameter, the function will not return until the packet is
/// completely transmitted.
    ot_u16      holdtime;
    uart_element_t*  txpayload;
    
#   if (MPIPE_USE_ACKS)
    if (data_priority == MPIPE_Ack)) {
        uart.priority  = data_priority;
        ///@todo In this space, the ACK needs to be swapped to the queue
        goto mpipedrv_tx_GO;
    }
    ///@todo In this space, swap to the standard mpipe queue
#   endif

    holdtime = q_blocktime(mpipe.alp.outq);
    if (holdtime != 0) {
        return -holdtime;
    }
    
    /// Load Queue payload into tlist ring buffer
    /// Tlist RB doesn't store a copy, just references.
    holdtime                    = __MPIPE_TIMEOUT(q_length(mpipe.alp.outq));
    uart.tlist.size            += 1;
    uart.tlist.j                = (uart.tlist.j + 1) & (UART_RB_MAX-1);
    txpayload                   = &uart.tlist.rb[uart.tlist.j];
    txpayload->front            = mpipe.alp.outq->getcursor;
    txpayload->length           = q_span(mpipe.alp.outq);
    mpipe.alp.outq->getcursor   = mpipe.alp.outq->putcursor;
    
    /// Don't start the TX if there's already activity, or if blocked.
    /// If there is already activity, the packet is queued in the RB
    /// and it will get sent in order of the RB.
    if (mpipe.state == MPIPE_Idle) {
        __SYS_CLKON();
        uart.tlist.i = (uart.tlist.i + 1) & (UART_RB_MAX-1);
        sub_txopen();
    }
    
    /// Purpose here is to continue blocking the queue until the
    /// most recently added packet (last packet) is sent.
    q_blockwrite(mpipe.alp.outq, blocking ? holdtime : 0);
    
    return holdtime;
}
#endif




ot_u16 mpipedrv_txsync() {
/// Send a single FF byte and return the timeout (We have it at 6).
/// We don't care about interrupts, because it is just one byte.
    sub_mpipe_open();
    mpipe.state = MPIPE_Idle;
    __UART_TXOPEN();
    MPIPE_UART->TDR = 0xFF;
    return 6;
}





#ifndef EXTF_mpipedrv_rx
void mpipedrv_rx(ot_bool blocking, mpipe_priority data_priority) {
#if (BOARD_FEATURE(MPIPE_BREAK))
    /// Wait for Line Sync.  Sync interrupt will call sub_rx()
    sub_mpipe_close();
    EXTI->IMR  |= MPIPE_UART_RXPIN;
#else
    /// Wait for character Sync.
    mpipe_rxsync_isr();
#endif

    //MPIPE_UART->CR1   = (USART_CR1_UE | USART_CR1_RE);
    //__DMA_RXOPEN(&uart.rxbuffer[0], uart.rxbuffer[0]+2);   //
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
    //ot_bool tx_process = False;
    ot_int  error_code = -1;

    __DMA_ALL_CLOSE();
    __DMA_ALL_CLEAR();

    switch (mpipe.state) {
        case MPIPE_Idle: //note, case doesn't break!

        case MPIPE_RxHeader: {
            ot_u16 blockticks;
            
            // If there is no payload or if the input queue is being used by 
            // someone else, this packet is registered as an error.
            uart.rxplen = PLATFORM_ENDIAN16(uart.header.plen);
            if (uart.rxplen == 0)                           error_code = -1;
            else if (q_blocktime(mpipe.alp.inq))            error_code = -11;
            else if (q_space(mpipe.alp.inq) < uart.rxplen)  error_code = -7;
            
            // No error, start receiving packet formally
            // Most important first thing is to reset DMA to grab first frame.
            else {
                blockticks = __MPIPE_TIMEOUT(uart.rxplen);
                q_blockwrite(mpipe.alp.inq, blockticks);
                
                uart.rxframelen = (uart.rxplen <= MPIPE_BUFFER_SIZE) ? uart.rxplen : MPIPE_BUFFER_SIZE;
                __DMA_RXOPEN(uart.rxbuffer, uart.rxframelen);
                mpipeevt_rxdetect(blockticks);
                mpipe.state = MPIPE_RxPayload;
                
                // Start-up the CRC streamer manually.
                // uart.crc.count is also used to track the bytes left to receive
                uart.crc.count  = uart.rxplen;
                uart.crc.val    = crc16drv_init();
                if ((uart.header.ctl & MPIPE_CTL_NOCRC) == 0) {
                    uart.crc.writeout   = False;
                    uart.crc.cursor     = (ot_u8*)&uart.header.plen;
                    uart.crc.count     += 4;
                    crc_calc_nstream(&uart.crc, 4);
                    uart.crc.cursor     = mpipe.alp.inq->putcursor;
                }
                return;             // Wait for next DMA RX interrupt
            }
        } goto mpipedrv_isr_RXSIG;  // handle error

        case MPIPE_RxPayload: {
            ot_u16  nextframe;
            
            // If there are more frames after this one, refresh DMA buffering
            if (uart.crc.count > MPIPE_BUFFER_SIZE) {
                nextframe = uart.crc.count - uart.rxframelen;
                if (nextframe > MPIPE_BUFFER_SIZE) {
                    nextframe = MPIPE_BUFFER_SIZE;
                }
                __DMA_RXOPEN(&uart.rxbuffer, nextframe);
            }
            
            // Write the last frame data received to the app/alp queue.
            q_writestring(mpipe.alp.inq, uart.rxbuffer, uart.rxframelen);
            
            // Do CRC if required, else manually adjust crc.count
            if ((uart.header.ctl & MPIPE_CTL_NOCRC) == 0) {
                crc_calc_nstream(&uart.crc, uart.rxframelen);
            }
            else {
                uart.crc.count -= uart.rxframelen;
            }
            
            // Framelen now becomes length of the frame underway
            uart.rxframelen = nextframe;
            
            // Update the hold time on the main queue.  This is not required,
            // and it is a bit of a hack, but it is nice to do
            q_blockwrite(mpipe.alp.inq, __MPIPE_TIMEOUT(uart.crc.count));
            
            // Payload is not done being received.
            // Exit the ISR and wait for next DMA interrupt
            if (uart.crc.count > 0) {
                return;
            }
            
            // Packet is done being received: check CRC and, if invalid,
            // retract the rx queue putcursor
            if (PLATFORM_ENDIAN16(uart.crc.val) == uart.header.crc16) {
                error_code = 0;   
            }
            else {
                mpipe.alp.inq->putcursor -= uart.rxplen;
                error_code = -2;
            }
#           if (MPIPE_USE_ACKS)
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
#           endif    
        } goto mpipedrv_isr_RXSIG;

#       if (MPIPE_USE_ACKS)
        case MPIPE_TxAck_Wait:
            __UART_CLEAR();
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_TE | USART_CR1_TCIE);
            return;

        case MPIPE_TxAck_Done: {  // TX'ed an ACK
            ///@todo Put Ack-Tx in the rxbuffer
            if (uart.rxbuffer[3] != 0) { // TX'ed a NACK
                mpipedrv_rx(False, uart.priority);
                mpipe.state = MPIPE_RxHeader;
                return;
            }
            uart.priority = MPIPE_Low;
        } goto mpipedrv_isr_RXSIG;
#       endif

        case MPIPE_Tx_Wait:
            mpipe.state = MPIPE_Tx_Done;
            sub_txcont();
            return;
        
        case MPIPE_Tx_Done:
            __UART_CLEAR();
            MPIPE_UART->CR1 = (USART_CR1_UE | USART_CR1_TE | USART_CR1_TCIE);
            return; 
            
#       if (MPIPE_USE_ACKS)
        case MPIPE_RxAck:
            
            //RX'ed NACK
            if (crc16drv_block(uart.rxbuffer, 8) != 0) {
                ///@todo retransmit
                //mpipedrv_tx(False, uart.priority);
                return;
            }
            
            //RX'ed an ACK.  Remove Packet from ringbuffer
            uart.tlist.size--;
            mpipedrv_rx(False, 0);
            mpipeevt_txdone(0);
            return;
#       endif
       
       // Normal behavior is to fall through.  The default case could be removed,
       // but it gets compiled-out and we leave to suppress warnings.
       default: break; //goto mpipedrv_isr_TXSIG;  
    }

    // The RX process is complete, including and ACKing.
    // - Close MPipe and call rxdone event handler from MPipe Task
    // - If RX CRC matters, then make sure to compute it.
    mpipedrv_isr_RXSIG:
    mpipedrv_rx(False, 0);
    mpipeevt_rxdone(error_code);
}

#endif








#endif

