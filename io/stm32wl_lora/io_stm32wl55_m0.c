/* Copyright 2016 JP Norair
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
  * @file       /platform/stm32wl_lora/io_stm32wl55_m0.c
  * @author     JP Norair
  * @version    R100
  * @date       31 Jan 2022
  * @brief      STM32WL Lora transceiver interface implementation for STM32WL
  * @ingroup    STM32WL_LoRa
  *
  * @todo this file is a very incomplete port from SX127x to STM32WL LoRa.
  *       It is only used right now for compiling platform tests, and it
  *       will be overhauled shortly. 
  ******************************************************************************
  */

#include <otplatform.h>
#if (OT_FEATURE(M2) && defined(__STM32WL_LORA__))

#include <otlib/utils.h>
#include <otlib/delay.h>
#include <otsys/types.h>
#include <otsys/config.h>
#include <m2/dll.h>

#include <io/stm32wl_lora/config.h>
#include <io/stm32wl_lora/interface.h>

//For test
#include <otlib/logger.h>



/// SPI Bus Macros:
/// Most are straightforward, but take special note of the clocking macros.
/// On STM32, the peripheral clock must be enabled for the peripheral to work.
/// There are clock-enable bits for active and low-power mode.  Both should be
/// enabled before SPI usage, and both disabled afterward.
#if (RADIO_SPI_ID == 3)
#   define _SPICLK              ((PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB1CLKDIV)/2)
#   define _SPI_IRQ             SUBGHZSPI_IRQn
#   define _DMA_UNIT            DMA2
#   define _DMAMUXRX            DMAMUX1_Channel12
#   define _DMAMUXTX            DMAMUX1_Channel13
#   define _DMA_ISR             platform_dma2ch5_isr
#   define _DMARX               DMA2_Channel5
#   define _DMATX               DMA2_Channel6
#   define _DMA_IRQ             DMA2_DMAMUX1_OVR_IRQn
#   define _DMARX_IFG           (0x2 << (4*(5-1)))
#   define _DMATX_IFG           (0x2 << (4*(6-1)))
#   define _DMA_RXREQ_ID        41
#   define _DMA_TXREQ_ID        42
#   define __DMA_CLEAR_IFG()    (DMA2->IFCR = (0xF << (4*(5-1))) | (0xF << (4*(6-1))))
#   define __SPI_CLKON()        (RCC->C2APB3ENR |= RCC_C2APB3ENR_SUBGHZSPIEN)
#   define __SPI_CLKOFF()       (RCC->C2APB3ENR &= ~RCC_C2APB3ENR_SUBGHZSPIEN)

#else
#   error "RADIO_SPI_ID is misdefined, must be SPI3 (SUBGHZSPI)."
#endif

/// SPI clock = bus clock/2
/// It's not clear what the limits of it are, either from STM32WL or SX126x
/// datasheets.  So, we run it at 24 MHz.  It can also be set to 12 or 6 MHz.
#if (_SPICLK > 24000000)
#   define _SPI_DIV (1<<SPI_CR1_BR_Pos)
#else
#   define _SPI_DIV (0<<SPI_CR1_BR_Pos)
#endif


#define __DMA_CLEAR_IRQ()  (NVIC_ClearPendingIRQ(_DMA_IRQ))
#define __DMA_ENABLE()     do { \
                                _DMARX->CCR     = (DMA_CCR_MINC | (3<<DMA_CCR_PL_Pos) | DMA_CCR_TCIE | DMA_CCR_EN); \
                                _DMATX->CCR    |= (DMA_CCR_DIR | DMA_CCR_MINC | (3<<DMA_CCR_PL_Pos) | DMA_CCR_EN); \
                            } while(0)
#define __DMA_DISABLE()    do { \
                                _DMARX->CCR     = 0; \
                                _DMATX->CCR     = 0; \
                            } while(0)

#define __SPI_CS_HIGH()     RADIO_SPICS_PORT->BSRR = (ot_u32)RADIO_SPICS_PIN
#define __SPI_CS_LOW()      RADIO_SPICS_PORT->BRR  = (ot_u32)RADIO_SPICS_PIN
#define __SPI_CS_ON()       PWR->SUBGHZSPICR = 0
#define __SPI_CS_OFF()      PWR->SUBGHZSPICR = PWR_SUBGHZSPICR_NSS

#define __SPI_ENABLE()      do { \
                                RADIO_SPI->CR2 = (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN); \
                                RADIO_SPI->SR  = 0; \
                                RADIO_SPI->CR1 = (SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_MSTR | _SPI_DIV | SPI_CR1_SPE); \
                            } while(0)

#define __SPI_DISABLE()     (RADIO_SPI->CR1 = (SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_MSTR | _SPI_DIV))
#define __SPI_GET(VAL)      VAL = RADIO_SPI->DR
#define __SPI_PUT(VAL)      RADIO_SPI->DR = VAL




/** Embedded Interrupts <BR>
  * ========================================================================<BR>
  * None: The Radio core only uses the GPIO interrupts, which must be handled
  * universally in platform/core_main.c due to the multiplexed nature of
  * the EXTI system.  However, the DMA RX complete EVENT is used by the SPI
  * engine.  EVENTS are basically a way to sleep where you would otherwise
  * need to use busywait loops.  ARM Cortex-M takes all 3 points.
  */




/** Pin Check Functions <BR>
  * ========================================================================
  * @todo make sure this is all working
  */

// Ready Pin always on DIO5
#if defined(_READY_PIN)
inline ot_uint wllora_readypin_ishigh(void)   { 
    return 1;
    //return (_READY_PORT->IDR & _READY_PIN); 
}
#endif

// CAD-Detect may be implemented on DIO1
inline ot_uint wllora_cadpin_ishigh(void)     { 
    return 1;
    //return (_CAD_DETECT_PORT->IDR & _CAD_DETECT_PIN); 
}






ot_u8 wllora_getbasepwr() {
/// Base Power code: 0-3.  For this SX127x impl it's always 3.
    return 3;
}



/** Bus interface <BR>
  * ========================================================================
  */
void wllora_reset() {
/// @note this uses a busywait loop.  A timed WFE could be used in the test
///       loop IFF:
///       - wllora_reset() is guaranteed never to run in an ISR
///       - profiling of the reset sequence shows value in using WFE.
    ot_u32 test;
    do {
        test        = RCC->CSR & RCC_CSR_RFRSTF;
        RCC->CSR   ^= RCC_CSR_RFRST;
        while ((RCC->CSR & RCC_CSR_RFRSTF) == test);
    } while (test == 0);
}


void wllora_init_bus() {

    ///1. Reset the Radio Core
    wllora_reset();
    
    ///2. Set-up DMA to work with SPI.  The DMA is bound to the SPI and it is
    ///   used for Duplex TX+RX.  The DMA RX Channel is used as an EVENT.  The
    ///   STM32WL can do in-context naps using EVENTS.  To enable the EVENT, we
    ///   enable the DMA RX interrupt bit, but not the NVIC.
     BOARD_DMA_CLKON();
    _DMAMUXRX->CCR  = _DMA_RXREQ_ID;
    _DMAMUXTX->CCR  = _DMA_TXREQ_ID;
    _DMARX->CMAR    = (ot_u32)&wllora.busrx[-1];
    _DMARX->CPAR    = (ot_u32)&RADIO_SPI->DR;
    _DMATX->CPAR    = (ot_u32)&RADIO_SPI->DR;
     BOARD_DMA_CLKOFF();

    /// 3. The STM32WL has a single interrupt for the RF Core itself.  After
    ///    CPU gets this IRQ, it needs to look at Radio Core IRQ registers to
    ///    see the IRQ mask itself.
    PWR->C2CR3 |= PWR_C2CR3_EWRFIRQ;
    NVIC_SetPriority(SUBGHZ_Radio_IRQn, PLATFORM_NVIC_RF_GROUP);
    NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
    
    /// 4. Put RF Core to sleep
    wllora_sleep(True);
}



void wllora_spibus_wait() {
/// Blocking wait for SPI bus to be over
     while (RADIO_SPI->SR & SPI_SR_BSY);
}




void wllora_spibus_io(ot_u8 cmd_len, ot_u8 resp_len, const ot_u8* cmd) {
///@note BOARD_DMA_CLKON() must be defined in the board support header as a
/// macro or inline function.  As the board may be using DMA for numerous
/// peripherals, we cannot assume in this module if it is appropriate to turn-
/// off the DMA for all other modules.

     platform_disable_interrupts();
     __SPI_CLKON();
     __SPI_ENABLE();
     __SPI_CS_ON();

    /// Set-up DMA, and trigger it.  TX goes out from parameter.  RX goes into
    /// module buffer.  If doing a read, the garbage data getting duplexed onto
    /// TX doesn't affect the SX127x.  If doing a write, simply disregard the
    /// RX duplexed data.
    BOARD_RFSPI_CLKON();
    BOARD_DMA_CLKON();
    __DMA_CLEAR_IFG();
    cmd_len        += resp_len;
    _DMARX->CNDTR   = cmd_len;
    _DMATX->CNDTR   = cmd_len;
    _DMATX->CMAR    = (ot_u32)cmd;
    __DMA_ENABLE();

    /// WFE is only reliable on EXTI line interrupts, as far as I can test.
    /// So do busywait until DMA is done RX-ing
    //do { __WFE(); }
    while((_DMA_UNIT->ISR & _DMARX_IFG) == 0);
    __DMA_CLEAR_IRQ();
    __DMA_CLEAR_IFG();
    __DMA_DISABLE();

    /// Turn-off and disable SPI to save energy
    __SPI_CS_OFF();
    __SPI_DISABLE();
    __SPI_CLKOFF();
    BOARD_DMA_CLKOFF();
    BOARD_RFSPI_CLKOFF();
    platform_enable_interrupts();
}






/** Common GPIO setup & interrupt functions  <BR>
  * ========================================================================<BR>
  * Your radio ISR function should be of the type void radio_isr(ot_u8), as it
  * will be a soft ISR.  The input parameter is an interrupt vector.  The vector
  * values are shown below:
  *
  * -------------- RX MODES (set wllora_iocfg_rx()) --------------
  * IMode = 0       CAD Done:                   0
  * (Listen)        CAD Detected:               -
  *                 Hop (Unused)                -
  *                 Valid Header:               -
  *
  * IMode = 1       RX Done:                    1
  * (RX Data)       RX Timeout:                 2
  *                 Hop (Unused)                -
  *                 Valid Header:               4
  *
  * -------------- TX MODES (set wllora_iocfg_tx()) --------------
  * IMode = 5       CAD Done:                   5   (CCA done)
  * (CSMA)          CAD Detected:               -   (0/1 = pass/fail)
  *                 Hop (Unused)                -
  *                 Valid Header                -
  *
  * IMode = 6       TX Done:                    6
  * (TX)            
  */

void wllora_antsw_off(void) {
    BOARD_RFANT_OFF();
}

void wllora_antsw_on(void) {
    BOARD_RFANT_ON();
}

void wllora_antsw_tx(ot_bool use_paboost) {
    wllora_antsw_on();
    BOARD_RFANT_TX(use_paboost);
}

void wllora_antsw_rx(void) {
    wllora_antsw_on();
    BOARD_RFANT_RX();
}




void wllora_int_config(ot_u32 ie_sel) {
//     ot_u32 scratch;
//     EXTI->PR    = (ot_u32)RFI_ALL;
//     scratch     = EXTI->IMR;
//     scratch    &= ~((ot_u32)RFI_ALL);
//     scratch    |= ie_sel;
//     EXTI->IMR   = scratch;
}

inline void wllora_int_clearall(void) {
//     EXTI->PR = RFI_ALL;
}

inline void wllora_int_force(ot_u16 ifg_sel) { 
//     EXTI->SWIER |= (ot_u32)ifg_sel; 
}

inline void wllora_int_turnon(ot_u16 ie_sel) { 
//     EXTI->IMR   |= (ot_u32)ie_sel;  
}

inline void wllora_int_turnoff(ot_u16 ie_sel)  {
//     EXTI->PR    = (ot_u32)ie_sel;
//     EXTI->IMR  &= ~((ot_u32)ie_sel);
}




void wllora_wfe(ot_u16 ifg_sel) {
//     do {
//         __WFE();
//     }
//     while((EXTI->PR & ifg_sel) == 0);

    // clear IRQ value in SX127x by setting IRQFLAGS to 0xFF
    wllora_write(RFREG(LR_IRQFLAGS), 0xFF);

    // clear pending register(s)
//     EXTI->PR = ifg_sel;
}






#endif //#if from top of file

