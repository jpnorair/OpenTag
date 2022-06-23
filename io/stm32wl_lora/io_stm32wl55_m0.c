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
#   define _DMAMUXRX            DMAMUX1_Channel11
#   define _DMAMUXTX            DMAMUX1_Channel12
#   define _DMA_ISR             platform_dma2ch5_isr
#   define _DMARX               DMA2_Channel5
#   define _DMATX               DMA2_Channel6
#   define _DMA_IRQ             DMA2_DMAMUX1_OVR_IRQn
#   define _DMARX_IFG           (0x2 << (4*(5-1)))
#   define _DMATX_IFG           (0x2 << (4*(6-1)))
#   define _DMA_RXREQ_ID        41
#   define _DMA_TXREQ_ID        42
#   define __DMA_CLEAR_IFG()    (DMA2->IFCR = (0xF << (4*(5-1))) | (0xF << (4*(6-1))))
#   define __SPI_CLKON()        (RCC->C2APB3ENR = RCC_C2APB3ENR_SUBGHZSPIEN)
#   define __SPI_CLKOFF()       (RCC->C2APB3ENR = 0)

#else
#   error "RADIO_SPI_ID is misdefined, must be SPI3 (SUBGHZSPI)."
#endif

/// SPI clock = bus clock/2
/// It's not clear what the limits of it are, either from STM32WL or SX126x
/// datasheets.  So, we run it at 24 MHz.  It can also be set to 12 or 6 MHz.
#if (_SPICLK > 12000000)
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

#define __SPI_CS_HIGH()     (PWR->SUBGHZSPICR = PWR_SUBGHZSPICR_NSS)
#define __SPI_CS_LOW()      (PWR->SUBGHZSPICR = 0)
#define __SPI_CS_ON()       __SPI_CS_LOW()
#define __SPI_CS_OFF()      __SPI_CS_HIGH()

#define __SPI_ENABLE()      do { \
                                RADIO_SPI->CR2 = (SPI_CR2_FRXTH | (7<<8) | SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN); \
                                RADIO_SPI->SR  = 0; \
                                RADIO_SPI->CR1 = (SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_MSTR | _SPI_DIV | SPI_CR1_SPE); \
                            } while(0)

#define __SPI_DISABLE()     (RADIO_SPI->CR1 = (SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_MSTR | _SPI_DIV))
#define __SPI_GET(VAL)      VAL = RADIO_SPI->DR
#define __SPI_PUT(VAL)      RADIO_SPI->DR = VAL



static void sub_xtal_on(ot_bool blocking) {
    BOARD_HSXTAL_ON();

    ///@todo watchdog in the loop here
    if (blocking) {
        //while((RCC->CR & RCC_CR_HSERDY) == 0);
    }
}

static void sub_xtal_off(void) {
    BOARD_HSXTAL_OFF();
}





/** Driver Interrupts <BR>
  * ========================================================================<BR>
  * This driver optionally includes the RFBUSY interrupt.
  * It always includes the RFIRQ interrupt
  */

void platform_isr_rfbusy() {
///@todo Use the RF Busy interrupt to provide non blocking usage of SPI cmds.
}

void platform_isr_rfirq() {
///@todo I'm tempted to just mask the value of the IRQ DIO pins and configure
///      them carefully as in prior implementations.
    ot_u16 irq_mask;
    irq_mask = wllora_getirq_cmd();
    wllora_clrirq_cmd();

    // This function is generally implemented in radio_...c, because it will
    // need to interact with the radio control and tasking.
    wllora_virtual_isr(irq_mask);
}





// Not used with current WL implementation
//void wllora_irq0_isr() {   wllora_virtual_isr(wllora.imode);     }
//void wllora_irq1_isr() {   wllora_virtual_isr(wllora.imode + 1); }
//void wllora_irq2_isr() {   wllora_virtual_isr(wllora.imode + 2); }




/** MCU-side Interrupt Functions <BR>
  * ==========================================================================
  */

OT_WEAK inline void wllora_mcuirq_off() {
    PWR->C2CR3 &= ~PWR_C2CR3_EWRFIRQ;
}

OT_WEAK inline void wllora_mcuirq_on() {
    PWR->C2CR3 |= PWR_C2CR3_EWRFIRQ;
}

void wllora_mcuirq_config(ot_u32 ie_sel) {
//     ot_u32 scratch;
//     EXTI->PR    = (ot_u32)RFI_ALL;
//     scratch     = EXTI->IMR;
//     scratch    &= ~((ot_u32)RFI_ALL);
//     scratch    |= ie_sel;
//     EXTI->IMR   = scratch;
}

inline void wllora_mcuirq_clearall(void) {
//     EXTI->PR = RFI_ALL;
}

inline void wllora_mcuirq_force(ot_u16 ifg_sel) {
//     EXTI->SWIER |= (ot_u32)ifg_sel;
}

inline void wllora_mcuirq_turnon(ot_u16 ie_sel) {
//     EXTI->IMR   |= (ot_u32)ie_sel;
}

inline void wllora_mcuirq_turnoff(ot_u16 ie_sel)  {
//     EXTI->PR    = (ot_u32)ie_sel;
//     EXTI->IMR  &= ~((ot_u32)ie_sel);
}

void wllora_mcuirq_wfe(ot_u16 ifg_sel) {
//     do {
//         __WFE();
//     }
//     while((EXTI->PR & ifg_sel) == 0);

    // clear IRQ value in SX127x by setting IRQFLAGS to 0xFF
    //wllora_write(RFREG(LR_IRQFLAGS), 0xFF);

    // clear pending register(s)
//     EXTI->PR = ifg_sel;
}

inline ot_bool wllora_isbusy(void)   {
///@note The RFBUSYMS functionality is not as indicated in the Reference Manual.
///      In fact, it is only a validator that the RFBUSYS signal is OK to use.
///      Device is considered busy if *both* are 1.
    ot_uint test;
    test = PWR->SR2 & (PWR_SR2_RFBUSYMS | PWR_SR2_RFBUSYS);
    return (ot_bool)(test == (PWR_SR2_RFBUSYMS | PWR_SR2_RFBUSYS));
}





ot_u8 wllora_getbasepwr() {
/// Base Power code: 0-3.  For this WL impl it's always 3.
///@todo return 0 or 1 if we do a fully non-blocking impl and DMA/SPI is on.
    return 3;
}




/** Control interface <BR>
  * ========================================================================
  */

void wllora_reset() {
/// @note this uses a busywait loop.  A timed WFE could be used in the test
///       loop IFF:
///       - wllora_reset() is guaranteed never to run in an ISR
///       - profiling of the reset sequence shows value in using WFE.
    ot_u32 test;

    /// ahead of reset, the NSS must be high.
    __SPI_CS_OFF();
    wllora.state = RFSTATE_off;

    /// Guarantee the HSE/TCXO is off, before doing reset.
    sub_xtal_off();

    // (This 2-state loop will assure radio is reset)
    do {
        test        = RCC->CSR & RCC_CSR_RFRSTF;
        RCC->CSR   ^= RCC_CSR_RFRST;
        while ((RCC->CSR & RCC_CSR_RFRSTF) == test);
    } while (test == 0);

    /// After reset, the device is effectively in cold-sleep mode.  We want to
    /// allow it to calibrate itself, so we ensure the TCXO is on before
    /// driving NSS low to bring out of sleep.
    sub_xtal_on(True);

    /// Now, Drive NSS low and wait for chip to tell it is ready.  It should do
    /// a calibration during this wake-up time.
    __SPI_CS_ON();
    delay_us(1);
    while (wllora_isbusy());

    /// RF is now in RCstandby state, and calibrated.  Drive NSS high to
    /// complete the reset process.  Also, turn off the crystal at this point.
    wllora.state = RFSTATE_RCstandby;
    __SPI_CS_OFF();
    sub_xtal_off();
}


ot_u16 wllora_set_state(WLLora_State new_state, ot_bool blocking) {
///@todo implement a watchdog for each state.
///@todo implement the non-blocking feature, which returns the amount of time
///      (in ticks) the state change process is expected to take, and this
///      routine will need to run via an ISR.
///
    ot_u8 cfg_opt1;

    if (new_state == RFSTATE_off) {
        ///@todo implement this: not sure exactly how.
        return 0;
    }

    while (new_state != wllora.state) {
        switch (wllora.state) {
            /// From off, need to reset the device, which puts it into standby-13
            case RFSTATE_off:
                wllora_reset();
                break;

            /// From Cold sleep, need to drop NSS to wake-up, then it does
            /// calibration automatically and arrives in standby-13.
            /// From Warm sleep, the wakeup method is the same, but calibration
            /// does not occur.  Right now these are handled the same way.
            case RFSTATE_coldsleep:
                if (new_state == RFSTATE_warmsleep) {
                    // coldsleep -> warmsleep is invalid transition
                    return 0;
                }
                if (new_state < RFSTATE_RCstandby) {
                    if (new_state == RFSTATE_calibration) {
                        new_state = RFSTATE_RCstandby;
                    }
                    sub_xtal_on(True);
                }
            case RFSTATE_warmsleep:
                __SPI_CS_ON();
                delay_us(1);    // hold for at least 600ns to guarantee BUSY signal.
                while (wllora_isbusy());
                __SPI_CS_OFF();
                wllora.state = RFSTATE_RCstandby;
                break;

            /// Calibration case is only entered (in this function) as a result
            /// of setting new_state = calibration as an argument, which itself
            /// requires the system to be in standby.  So, we wait for
            /// calibration to finish and transfer to intermediate standby.
            case RFSTATE_calibration:
                while (wllora_isbusy());
                sub_xtal_off();
                wllora.state = RFSTATE_RCstandby;
                return 0;

            /// RCstandby and HSEstandby can go into any other mode.
            case RFSTATE_RCstandby:
            case RFSTATE_HSEstandby: {
                goto ENTER_ENDSTATE;
            }

            /// FS can go to standby modes, cad, rx, tx.
            /// CAD can go to standby modes, fs, rx, tx.
            /// RX can go to standby modes, fs, cad, tx.
            /// TX can go to standby modes, fs, cad, rx.
            case RFSTATE_fs:
            case RFSTATE_cad:
            case RFSTATE_rx:
            case RFSTATE_tx:
                // go to standby first if switching to a sleep mode
                if (new_state < RFSTATE_RCstandby) {
                    //wllora_standby_cmd(LR_STANDBYCFG_HSE32MHz);
                    //wllora.state = RFSTATE_HSEstandby;
                    wllora_standby_cmd(LR_STANDBYCFG_RC13MHz);
                    wllora.state = RFSTATE_RCstandby;

                    ///@note need to block until standby, here.
                    delay_us(1);    // hold for at least 600ns to guarantee BUSY signal.
                    while (wllora_isbusy());
                    break;
                }

                // If not switching to sleep, go directly to that mode.
                goto ENTER_ENDSTATE;
        }
    }

    return 0;


    ENTER_ENDSTATE:
    switch (new_state) {
        /// need to wait 500us after sleep.
        /// this is a blocking impl.
        case RFSTATE_coldsleep:
            cfg_opt1 = LR_SLEEPCFG_COLD;
            goto ENTER_SLEEP;
        case RFSTATE_warmsleep:
            cfg_opt1 = LR_SLEEPCFG_WARM;
            goto ENTER_SLEEP;

        /// In this impl's manual calibration, we don't care about
        /// RC64k calibration -- that is disabled.
        case RFSTATE_calibration: {
            ot_u8 metastate;
            sub_xtal_on(True);
            wllora_calibrate_cmd(0b01111110);
            metastate = (ot_u8)RFSTATE_calibration + (ot_u8)blocking;
            wllora.state = metastate;
        }   goto EXIT_TO_NEWSTATE;

        /// RCstandby can be entered via HSEstandby.
        case RFSTATE_RCstandby:
            wllora_standby_cmd(LR_STANDBYCFG_RC13MHz);
            wllora.state = RFSTATE_RCstandby;
            goto EXIT_TO_NEWSTATE;

        /// HSEstandby can be entered via RCstandby.
        case RFSTATE_HSEstandby:
            ///@todo validate this with False
            sub_xtal_on(/*True*/ False);
            wllora_standby_cmd(LR_STANDBYCFG_HSE32MHz);
            wllora.state = RFSTATE_HSEstandby;
            goto EXIT_TO_NEWSTATE;

        case RFSTATE_fs:
            ///@todo validate this with False
            sub_xtal_on(/*True*/ False);
            wllora_fs_cmd();
            wllora.state = RFSTATE_fs;
            goto EXIT_TO_NEWSTATE;

        case RFSTATE_cad:
            ///@todo validate this with False
            sub_xtal_on(/*True*/ False);
            wllora_cad_cmd();
            wllora.state = RFSTATE_cad;
            goto EXIT_TO_NEWSTATE;

        /// RX entry via this function assumes no timeout
        case RFSTATE_rx:
            ///@todo validate this with False
            sub_xtal_on(/*True*/ False);
            wllora_rx_cmd(0);
            //wllora_rx_cmd(0xFFFFFFFF );
            wllora.state = RFSTATE_rx;
            goto EXIT_TO_NEWSTATE;

        /// TX entry via this function assumes no timeout
        case RFSTATE_tx:
            ///@todo validate this with False
            sub_xtal_on(/*True*/ False);
            wllora_tx_cmd(0);
            wllora.state = RFSTATE_tx;
            goto EXIT_TO_NEWSTATE;

        default:
            return 0;
    }

    EXIT_TO_NEWSTATE:
    if (blocking) {
        ///@todo implement watchdog on the isbusy loop
        delay_us(1);    // hold for at least 600ns to guarantee BUSY signal.
        while(wllora_isbusy());
        if (wllora.state < RFSTATE_HSEstandby) {
            sub_xtal_off();
        }
    }
    ///@todo report the amount of time, not just 0
    return 0;

    ENTER_SLEEP:
    sub_xtal_off();
    wllora_sleep_cmd(cfg_opt1);
    ///@todo make non-blocking impl.  need to wait 500us after sleep before
    ///      accessing again the spi.
    delay_us(700);
    return 0;
}


ot_u16 wllora_until_ready(void) {
///@todo report amount of time before device is expected to be ready (not busy).
    return 0;
}






/** Bus interface <BR>
  * ========================================================================
  */

void wllora_init_bus() {

    ///1. Reset the Radio Core.
    ///   After wllora_reset() the radio is in RCstandby
    wllora_reset();

    ///2. Set-up DMA to work with SPI.  The DMA is bound to the SPI and it is
    ///   used for Duplex TX+RX.  The DMA RX Channel is used as an EVENT.  The
    ///   STM32WL can do in-context naps using EVENTS.  To enable the EVENT, we
    ///   enable the DMA RX interrupt bit, but not the NVIC.
     BOARD_DMA_CLKON();
    _DMAMUXRX->CCR  = _DMA_RXREQ_ID;
    _DMAMUXTX->CCR  = _DMA_TXREQ_ID;
    _DMARX->CMAR    = (ot_u32)&wllora.cmd.buf.rx[0];
    _DMARX->CPAR    = (ot_u32)&RADIO_SPI->DR;
    _DMATX->CPAR    = (ot_u32)&RADIO_SPI->DR;
     BOARD_DMA_CLKOFF();

    /// 3. The STM32WL has a single interrupt for the RF Core itself.  After
    ///    CPU gets this IRQ, it needs to look at Radio Core IRQ registers to
    ///    see the IRQ mask itself.
    //PWR->C2CR3 |= (PWR_C2CR3_EWRFIRQ | PWR_C2CR3_EWRFBUSY);
    //PWR->C2CR3 |= (PWR_C2CR3_EWRFIRQ);

    NVIC_SetPriority(SUBGHZ_Radio_IRQn, PLATFORM_NVIC_RF_GROUP);
    NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);

    ///@note Radio exits this function in RCstandby state
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

    ///@todo This needs to be much more sophisticated
    volatile ot_u16 count;
    count = 0;
    while (wllora_isbusy()) {
        count++;
    }

    platform_disable_interrupts();

    __SPI_CLKON();
    __SPI_ENABLE();
    __SPI_CS_ON();

    /// Set-up DMA, and trigger it.  TX goes out from parameter.  RX goes into
    /// module buffer.  If doing a read, the garbage data getting duplexed onto
    /// TX doesn't affect the SX127x.  If doing a write, simply disregard the
    /// RX duplexed data.
    BOARD_DMA_CLKON();
    __DMA_CLEAR_IFG();
    cmd_len        += resp_len;
    _DMARX->CNDTR   = cmd_len;
    _DMATX->CNDTR   = cmd_len;
    _DMATX->CMAR    = (ot_u32)cmd;
    __DMA_ENABLE();

    /// WFE is only reliable on EXTI line interrupts, as far as I can test.
    ///@todo could have WFE until WRFBUSY trigger, but only for blocking impl.
    /// So do busywait until DMA is done RX-ing
    //do { __WFE(); }
    while((_DMA_UNIT->ISR & _DMARX_IFG) == 0);

    /// After DMA transfer complete, immediately set NSS high (CS OFF) and shut
    /// off the DMA and SPI peripherals to save energy.
    __SPI_CS_OFF();

    ///@note 600ns must pass between this NSS->high and a successive entry to
    ///      the wllora_isbusy() line above in this function.  At 48 MHz, that
    ///      is roughly 29 clocks, or at most 29 instructions.  We have a
    ///      working assumption there is a minimum of 29 instructions between
    ///      this line and the wllora_isbusy() above.

    __DMA_CLEAR_IRQ();
    __DMA_CLEAR_IFG();
    __DMA_DISABLE();
    BOARD_DMA_CLKOFF();

    __SPI_DISABLE();
    __SPI_CLKOFF();


    platform_enable_interrupts();
}






/** Common GPIO setup & interrupt functions  <BR>
  * ========================================================================<BR>
  * - GPIO functions to enable Front-End components.
  * - Interrupt functions for managing EXTI interrupts mapped to radio DIOs.
  *   In the present implementation, these are not used.
  */

void wllora_antsw_off(void) {
    BOARD_RFANT_OFF();

#   if BOARD_FEATURE(RFSMPS)
    if (wllora_ext.smps_setting != WLLORA_SMPS_DEFAULT) {
        wllora_ext.smps_setting = WLLORA_SMPS_DEFAULT;
        wllora_wrreg(LR_SMPSC2R, WLLORA_SMPS_DEFAULT);
    }
#   endif
}


void wllora_antsw_on(void) {
    BOARD_RFANT_ON();
}


void wllora_antsw_tx(void) {
    wllora_antsw_on();

#   if BOARD_FEATURE(RFSMPS)
    {   ot_u8 new_setting;
        new_setting = wllora_ext.use_boost ? WLLORA_SMPS_DEFAULT : WLLORA_SMPS_MAX;
        if (wllora_ext.smps_setting != new_setting) {
            wllora_ext.smps_setting = new_setting;
            wllora_wrreg(LR_SMPSC2R, new_setting);
        }
    }
#   endif
#   if defined(__STM32WL_22dBm__) && defined(__STM32WL_15dBm__)
    {   ot_u8 new_setting;
        new_setting = wllora_ext.use_boost ? 0x38 : 0x18;
        if (wllora_ext.ocr_setting != new_setting) {
            wllora_ext.ocr_setting = new_setting;
            wllora_wrreg(LR_PAOCPR, new_setting);
        }
    }
#   endif

    BOARD_RFANT_TX(wllora_ext.use_boost);
}


void wllora_antsw_rx(void) {
    wllora_antsw_on();

#   if BOARD_FEATURE(RFSMPS)
    if (wllora_ext.smps_setting != WLLORA_SMPS_DEFAULT) {
        wllora_ext.smps_setting = WLLORA_SMPS_DEFAULT;
        wllora_wrreg(LR_SMPSC2R, WLLORA_SMPS_DEFAULT);
    }
#   endif

    BOARD_RFANT_RX();
}



#endif //#if from top of file

