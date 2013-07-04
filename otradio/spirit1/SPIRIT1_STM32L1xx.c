/* Copyright 2009-2011 JP Norair
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
  * @file       /otradio/spirit1/SPIRIT1_STM32L1xx.h
  * @author     JP Norair
  * @version    R100
  * @date       4 Jan 2013
  * @brief      SPIRIT1 transceiver interface implementation for STM32L1xx
  * @ingroup    SPIRIT1
  *
  ******************************************************************************
  */

#include "OT_platform.h"
#if 1 //defined(PLATFORM_STM32L1xx)

#include "OT_utils.h"
#include "OT_types.h"
#include "OT_config.h"


#include "SPIRIT1_interface.h"

//For test
#include "OTAPI.h"


#ifdef RADIO_DB_ATTENUATION
#   define _ATTEN_DB    RADIO_DB_ATTENUATION
#else
#   define _ATTEN_DB    6
#endif




/// RF IRQ GPIO Macros:
#if (RADIO_IRQ3_SRCLINE < 0)
#   define _READY_PORT  RADIO_IRQ3_PORT
#   define _READY_PIN   RADIO_IRQ3_PIN
#   undef _RFIRQ3
#elif (RADIO_IRQ3_SRCLINE < 5)
#   define _RFIRQ3  (EXTI0_IRQn + RADIO_IRQ3_SRCLINE)
#elif ((RADIO_IRQ3_SRCLINE < 10) && !defined(_EXTI9_5_USED))
#   define _EXTI9_5_USED
#   define _RFIRQ3  (EXTI9_5_IRQn)
#elif !defined(_EXTI15_10_USED)
#   define _EXTI15_10_USED
#   define _RFIRQ3  (EXTI15_10_IRQn)
#endif



#if (RADIO_IRQ0_SRCLINE < 0)
#   undef _READY_PORT
#   undef _READY_PIN
#   define _READY_PORT  RADIO_IRQ2_PORT
#   define _READY_PIN   RADIO_IRQ2_PIN
#   undef _RFIRQ0
#elif (RADIO_IRQ0_SRCLINE < 5)
#   define _RFIRQ0  (EXTI0_IRQn + RADIO_IRQ0_SRCLINE)
#elif (RADIO_IRQ0_SRCLINE < 10)
#   define _EXTI9_5_USED
#   define _RFIRQ0  (EXTI9_5_IRQn)
#else
#   define _EXTI15_10_USED
#   define _RFIRQ0  (EXTI15_10_IRQn)
#endif

#if (RADIO_IRQ1_SRCLINE < 0)
#   undef _READY_PORT
#   undef _READY_PIN
#   define _READY_PORT  RADIO_IRQ1_PORT
#   define _READY_PIN   RADIO_IRQ1_PIN
#   undef _RFIRQ1
#elif (RADIO_IRQ1_SRCLINE < 5)
#   define _RFIRQ1  (EXTI0_IRQn + RADIO_IRQ1_SRCLINE)
#elif ((RADIO_IRQ0_SRCLINE < 10) && !defined(_EXTI9_5_USED))
#   define _EXTI9_5_USED
#   define _RFIRQ1  (EXTI9_5_IRQn)
#elif !defined(_EXTI15_10_USED)
#   define _EXTI15_10_USED
#   define _RFIRQ1  (EXTI15_10_IRQn)
#endif

#if (RADIO_IRQ2_SRCLINE < 0)
#   undef _READY_PORT
#   undef _READY_PIN
#   define _READY_PORT  RADIO_IRQ0_PORT
#   define _READY_PIN   RADIO_IRQ0_PIN
#   undef _RFIRQ2
#elif (RADIO_IRQ2_SRCLINE < 5)
#   define _RFIRQ2  (EXTI0_IRQn + RADIO_IRQ2_SRCLINE)
#elif ((RADIO_IRQ2_SRCLINE < 10) && !defined(_EXTI9_5_USED))
#   define _EXTI9_5_USED
#   define _RFIRQ2  (EXTI9_5_IRQn)
#elif !defined(_EXTI15_10_USED)
#   define _EXTI15_10_USED
#   define _RFIRQ2  (EXTI15_10_IRQn)
#endif





/// SPI Bus Macros: 
/// Most are straightforward, but take special note of the clocking macros.
/// On STM32L, the peripheral clock must be enabled for the peripheral to work.
/// There are clock-enable bits for active and low-power mode.  Both should be
/// enabled before SPI usage, and both disabled afterward.
#if (RADIO_SPI_ID == 1)
#   define _SPICLK          (PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB2CLKDIV)
#   define _UART_IRQ        SPI1_IRQn
#   define _DMA_ISR         platform_dma1ch2_isr
#   define _DMARX           DMA1_Channel2
#   define _DMATX           DMA1_Channel3
#   define _DMARX_IRQ       DMA1_Channel2_IRQn
#   define _DMATX_IRQ       DMA1_Channel3_IRQn
#   define _DMARX_IFG       (0x2 << (4*(2-1)))
#   define _DMATX_IFG       (0x2 << (4*(3-1)))
#   define __DMA_CLEAR_IFG() (DMA1->IFCR = (0xF << (4*(2-1))) | (0xF << (4*(3-1))))
#   define __SPI_CLKON()    (RCC->APB2ENR |= RCC_APB2ENR_SPI1EN)
#   define __SPI_CLKOFF()   (RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN)

#elif (RADIO_SPI_ID == 2)
#   define _SPICLK          (PLATFORM_HSCLOCK_HZ/BOARD_PARAM_APB1CLKDIV)
#   define _UART_IRQ        SPI2_IRQn
#   define _DMA_ISR         platform_dma1ch4_isr
#   define _DMARX           DMA1_Channel4
#   define _DMATX           DMA1_Channel5
#   define _DMARX_IRQ       DMA1_Channel4_IRQn
#   define _DMATX_IRQ       DMA1_Channel5_IRQn
#   define _DMARX_IFG       (0x2 << (4*(4-1)))
#   define _DMATX_IFG       (0x2 << (4*(5-1)))
#   define __DMA_CLEAR_IFG() (DMA1->IFCR = (0xF << (4*(4-1))) | (0xF << (4*(5-1))))
#   define __SPI_CLKON()    (RCC->APB1ENR |= RCC_APB1ENR_SPI2EN)
#   define __SPI_CLKOFF()   (RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN)

#else
#   error "RADIO_SPI_ID is misdefined, must be 1 or 2."

#endif

/// Set-up SPI peripheral for Master Mode, Full Duplex, DMAs used for TRX.
/// Configure SPI clock divider to make sure that clock rate is not above
/// 10MHz.  SPI clock = bus clock/2, so only systems that have HSCLOCK
/// above 20MHz need to divide.
#   if (_SPICLK > 20000000)
#       define _SPI_DIV (1<<3)
#   else
#       define _SPI_DIV (0<<3)
#   endif


#define __DMA_CLEAR_IRQ()  (NVIC->ICPR[(ot_u32)(_DMARX_IRQ>>5)] = (1 << ((ot_u32)_DMARX_IRQ & 0x1F)))
#define __DMA_ENABLE()     do { \
                                _DMARX->CCR = (DMA_CCR1_MINC | DMA_CCR1_PL_VHI | DMA_CCR1_TCIE | DMA_CCR1_EN); \
                                _DMATX->CCR |= (DMA_CCR1_DIR | DMA_CCR1_MINC | DMA_CCR1_PL_VHI | DMA_CCR1_EN); \
                            } while(0)
#define __DMA_DISABLE()    do { \
                                _DMARX->CCR = 0; \
                                _DMATX->CCR = 0; \
                            } while(0)          

#define __SPI_CS_HIGH()     RADIO_SPICS_PORT->BSRRL = (ot_u32)RADIO_SPICS_PIN
#define __SPI_CS_LOW()      RADIO_SPICS_PORT->BSRRH = (ot_u32)RADIO_SPICS_PIN
#define __SPI_CS_ON()       __SPI_CS_LOW()
#define __SPI_CS_OFF()      __SPI_CS_HIGH()
                            
#define __SPI_ENABLE()      do { \
                                RADIO_SPI->CR2 = (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN); \
                                RADIO_SPI->SR  = 0; \
                                RADIO_SPI->CR1 = (SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_MSTR | _SPI_DIV | SPI_CR1_SPE); \
                            } while(0)
                                
#define __SPI_DISABLE()     (RADIO_SPI->CR1 = (SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_MSTR | _SPI_DIV))
#define __SPI_GET(VAL)      VAL = RADIO_SPI->DR
#define __SPI_PUT(VAL)      RADIO_SPI->DR = VAL




/** Module Data for radio driver interface <BR>
  * ========================================================================
  */
spirit1_struct spirit1;




/** Embedded Interrupts <BR>
  * ========================================================================<BR>
  * None: The Radio core only uses the GPIO interrupts, which must be handled
  * universally in platform_isr_STM32L1xx.c due to the multiplexed nature of 
  * the EXTI system.  However, the DMA RX complete EVENT is used by the SPI 
  * engine.  EVENTS are basically a way to sleep where you would otherwise
  * need to use busywait loops.  ARM Cortex-M takes all 3 points.
  */




/** Basic Control Functions <BR>
  * ========================================================================
  */
void spirit1_coredump() {
///debugging function to dump-out register values of RF core (not all are used)
    ot_u8 i = 0;
    ot_u8 regval;
    ot_u8 label[]   = { 'R', 'E', 'G', '_', 0, 0 };

    do {
        regval = spirit1_read(i);
        otutils_bin2hex(&label[4], &i, 1);
        otapi_log_msg(MSG_raw, 6, 1, label, &regval);
        //mpipedrv_wait();
        platform_swdelay_ms(5);
    }
    while (++i != 0);
}



void spirit1_load_defaults() {
/// The data ordering is: WRITE LENGTH, WRITE HEADER (0), START ADDR, VALUES
/// Ignore registers that are set later, are unused, or use the hardware default values.
    static const ot_u8 spirit1_defaults[] = {
        15, 0,  0x01,   DRF_ANA_FUNC_CONF0, 
                        RFGPO(READY), RFGPO(GND), RFGPO(GND), RFGPO(GND),
                        DRF_MCU_CK_CONF,
                        DRF_IF_OFFSET_ANA, DRF_SYNT3, DRF_SYNT2, DRF_SYNT1, 
                        DRF_SYNT0, DRF_CHSPACE, DRF_IF_OFFSET_DIG,
        3,  0,  0xB4,   DRF_XO_RCO_TEST,
        4,  0,  0x9E,   DRF_SYNTH_CONFIG1, DRF_SYNTH_CONFIG0,
        3,  0,  0x18,   DRF_PAPOWER0,
        6,  0,  0x1C,   DRF_FDEV0, DRF_CHFLT_LS, DRF_AFC2, DRF_AFC1, 
        5,  0,  0x25,   DRF_AGCCTRL1, DRF_AGCCTRL0, DRF_ANT_SELECT_CONF, 
        3,  0,  0x3A,   DRF_QI,
        3,  0,  0x41,   DRF_FIFO_CONFIG0,
        4,  0,  0x4F,   DRF_PCKT_FLT_OPTIONS, DRF_PROTOCOL2,
      //3,  0,  0x93,   RFINT_TX_FIFO_ERROR,
        5,  0,  0xA3,   DRF_DEM_ORDER, DRF_PM_CONFIG2, DRF_PM_CONFIG1,
        0   //Terminating 0
    };
    
    ot_u8* cursor;
    cursor = (ot_u8*)spirit1_defaults;

    while (*cursor != 0) {
        ot_u8 cmd_len   = *cursor++;
        ot_u8* cmd      = cursor;
        cursor         += cmd_len;
        spirit1_spibus_io(cmd_len, 0, cmd);
    }

    // Early debugging test to make sure data was written (look at first write block)
//    {
//        volatile ot_u8 test;
//        ot_u8 i;
//        for (i=0x01; i<=0x0D; ++i) {
//            test = spirit1_read(i);
//        }
//    }
}



void spirit1_shutdown() {
/// Raise the Shutdown Line
    RADIO_SDN_PORT->BSRRL = RADIO_SDN_PIN;
}


void spirit1_reset() {
/// Turn-off interrupts, send Reset strobe, and wait for reset to finish.
    spirit1_int_turnoff(RFI_ALL);
    spirit1_strobe(STROBE(SRES));
    spirit1_waitforreset();
}


void spirit1_waitforreset() {
/// Wait for POR signal to rise.  
/// Save non-blocking implementation for a rainy day.
    while ((RADIO_IRQ0_PORT->IDR & RADIO_IRQ0_PIN) == 0);
}


void spirit1_waitforready() {
/// Wait for the Ready Pin to go high (reset pin is remapped in init).
/// STANDBY->READY should take about 75us
    while ((_READY_PORT->IDR & _READY_PIN) == 0);
}


ot_u16 spirit1_getstatus() {
/// Status is sent during every SPI access, so refresh it manually by doing a
/// dummy read, and then returning the global status data that is obtained 
/// during the read process.
    spirit1_read(0);
    return spirit1.status;
}



ot_u16 spirit1_mcstate() { 
    static const ot_u8 cmd[2] = { 1, RFREG(MC_STATE1) };
    spirit1_spibus_io(2, 2, (ot_u8*)cmd);
    return (ot_u16)*((ot_u16*)spirit1.busrx);
}


ot_u8   spirit1_ldctime()       { return spirit1_read( RFREG(TIMERS2) ); }
ot_u8   spirit1_ldcrtime()      { return spirit1_read( RFREG(TIMERS0) ); }
ot_u8   spirit1_rxtime()        { return spirit1_read( RFREG(TIMERS4) ); }
ot_u8   spirit1_rxbytes()       { return spirit1_read( RFREG(LINEAR_FIFO_STATUS0) ); }
ot_u8   spirit1_txbytes()       { return spirit1_read( RFREG(LINEAR_FIFO_STATUS1) ); }
ot_u8   spirit1_rssi()          { return spirit1_read( RFREG(RSSI_LEVEL) ); }



void spirit1_linkinfo(spirit1_link* link) {
    static const ot_u8 cmd[2] = { 0x01, RFREG(LINK_QUALIF2) };
    
    // Do Read command on LINK_QUALIF[2:0] and store results in link structure
    spirit1_spibus_io(2, 3, (ot_u8*)cmd);
    
    // Convert 3 byte SPIRIT1 output into 4 byte data structure
    link->pqi   = spirit1.busrx[0];
    link->sqi   = spirit1.busrx[1] & ~0x80;
    link->lqi   = spirit1.busrx[2] >> 4;
    link->agc   = spirit1.busrx[2] & 0x0f;
}









/** Bus interface (SPI + 2x GPIO) <BR>
  * ========================================================================
  */

void spirit1_init_bus() {
/// @note platform_init_periphclk() should have alread enabled RADIO_SPI clock
/// and GPIO clocks
    ot_u16 scratch;
    
    ///0. Preliminary Stuff
#   if (BOARD_FEATURE_RFXTALOUT)
    spirit1.clkreq = False;
#   endif
    
    ///1. Assure that Shutdown Line is Low
    RADIO_SDN_PORT->BSRRH   = RADIO_SDN_PIN;        // Clear
    
    ///2. Set-up DMA to work with SPI.  The DMA is bound to the SPI and it is
    ///   used for Duplex TX+RX.  The DMA RX Channel is used as an EVENT.  The
    ///   STM32L can do in-context naps using EVENTS.  To enable the EVENT, we
    ///   enable the DMA RX interrupt bit, but not the NVIC.
    BOARD_DMA_CLKON();
    _DMARX->CMAR    = (ot_u32)&spirit1.status;
    _DMARX->CPAR    = (ot_u32)&RADIO_SPI->DR;
    _DMATX->CPAR    = (ot_u32)&RADIO_SPI->DR;
    BOARD_DMA_CLKOFF();
    
    // Don't enable NVIC, because we want an EVENT, not an interrupt.
    //NVIC->IP[(ot_u32)_DMARX_IRQ]        = PLATFORM_NVIC_RF_GROUP;
    //NVIC->ISER[(ot_u32)(_DMARX_IRQ>>5)] = (1 << ((ot_u32)_DMARX_IRQ & 0x1F));

    /// 3. Connect GPIOs from SPIRIT1 to STM32L External Interrupt sources
    /// The GPIO configuration should be done in BOARD_PORT_STARTUP() and the
    /// binding of each GPIO to the corresponding EXTI should be done in
    /// BOARD_EXTI_STARTUP().  This architecture is required because the STM32L
    /// External interrupt mechanism must be shared by all drivers, and we can
    /// only know this information at the board level.
    ///
    /// However, here we set the EXTI lines to the rising edge triggers we need
    /// and configure the NVIC.  Eventually, the NVIC stuff might be done in
    /// the platform module JUST FOR EXTI interrupts though.
    
    EXTI->PR    =  RFI_ALL;         //clear flag bits
    EXTI->IMR  &= ~RFI_ALL;         //clear interrupt enablers
    EXTI->EMR  &= ~RFI_ALL;         //clear event enablers
    
    // PIN0 is falling edge, 1 & 2 are rising edge  
    EXTI->FTSR |= (RFI_SOURCE0 /*| RFI_SOURCE1*/);
    EXTI->RTSR |= (RFI_SOURCE1 | RFI_SOURCE2);
    
    NVIC->IP[(uint32_t)_RFIRQ0]         = (PLATFORM_NVIC_RF_GROUP << 4);
    NVIC->ISER[((uint32_t)_RFIRQ0>>5)]  = (1 << ((uint32_t)_RFIRQ0 & 0x1F));
#   ifdef _RFIRQ1
    NVIC->IP[(uint32_t)_RFIRQ1]         = (PLATFORM_NVIC_RF_GROUP << 4);
    NVIC->ISER[((uint32_t)_RFIRQ1>>5)]  = (1 << ((uint32_t)_RFIRQ1 & 0x1F));
#   endif
#   ifdef _RFIRQ2
    NVIC->IP[(uint32_t)_RFIRQ2]         = (PLATFORM_NVIC_RF_GROUP << 4);
    NVIC->ISER[((uint32_t)_RFIRQ2>>5)]  = (1 << ((uint32_t)_RFIRQ2 & 0x1F));
#   endif
#   ifdef _RFIRQ3
    NVIC->IP[(uint32_t)_RFIRQ3]         = (PLATFORM_NVIC_RF_GROUP << 4);
    NVIC->ISER[((uint32_t)_RFIRQ3>>5)]  = (1 << ((uint32_t)_RFIRQ3 & 0x1F));
#   endif
    
    ///4. The best way to wait for the SPIRIT1 to start is to wait for the reset
    ///   line to come high.  Once it is high, the chip is in ready.
    spirit1_waitforreset();

    ///5. Take GPIOs out of pull-down and into floating mode.  This is to save
    ///   energy, which could otherwise just be draining over the pull-downs.
    ///@todo this
    
    ///6. Put the SPIRIT1 into a default IO configuration, and then to sleep.
    ///   It is important to expose the READY signal on GPIO0, because the 
    ///   driver needs this signal to confirm state changes.
}



void spirit1_spibus_wait() {
/// Blocking wait for SPI bus to be over
    while (RADIO_SPI->SR & SPI_SR_BSY);
}




void spirit1_spibus_io(ot_u8 cmd_len, ot_u8 resp_len, ot_u8* cmd) {
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
    /// TX doesn't affect the SPIRIT1.  If doing a write, simply disregard the 
    /// RX duplexed data.
    BOARD_DMA_CLKON();
    __DMA_CLEAR_IFG();
    cmd_len        += resp_len;
    _DMARX->CNDTR   = cmd_len;
    _DMATX->CNDTR   = cmd_len;
    _DMATX->CMAR    = (ot_u32)cmd;
    __DMA_ENABLE();
    
    /// Use Cortex-M WFE (Wait For Event) to hold until DMA is complete.  This
    /// is the CM way of doing a busywait loop, but turning off the CPU core.
    /// The while loop is for safety purposes, in case another event comes.
    //do {
        //__WFE();
    //} 
    while((DMA1->ISR & _DMARX_IFG) == 0);
    __DMA_CLEAR_IRQ();
    __DMA_CLEAR_IFG();
    __DMA_DISABLE();

    /// Turn-off and disable SPI to save energy
    __SPI_CS_OFF();
    __SPI_DISABLE();
    __SPI_CLKOFF();
    BOARD_DMA_CLKOFF();
    platform_enable_interrupts();
}


void spirit1_strobe(ot_u8 strobe) {
    ot_u8 cmd[2];
    cmd[0]  = 0x80;
    cmd[1]  = strobe;
    spirit1_spibus_io(2, 0, cmd);
}

ot_u8 spirit1_read(ot_u8 addr) {
    ot_u8 cmd[2];
    cmd[0]  = 1;
    cmd[1]  = addr;
    spirit1_spibus_io(2, 1, cmd);
    return spirit1.busrx[0];
}

void spirit1_burstread(ot_u8 start_addr, ot_u8 length, ot_u8* data) {
    ot_u8 cmd[2];
    cmd[0]  = 1;
    cmd[1]  = start_addr;
    spirit1_spibus_io(2, length, (ot_u8*)cmd);
    platform_memcpy(data, spirit1.busrx, length);
}

void spirit1_write(ot_u8 addr, ot_u8 data) {
    ot_u8 cmd[3];
    cmd[0]  = 0;
    cmd[1]  = addr;
    cmd[2]  = data;
    spirit1_spibus_io(3, 0, cmd);
}

void spirit1_burstwrite(ot_u8 start_addr, ot_u8 length, ot_u8* cmd_data) {
    cmd_data[0] = 0;
    cmd_data[1] = start_addr;
    spirit1_spibus_io((2+length), 0, cmd_data);
}













/** Advanced Configuration <BR>
  * ========================================================================<BR>
  */

ot_int spirit1_calc_rssi(ot_u8 encoded_value) {
/// From SPIRIT1 datasheet: "The measured power is reported in steps of half-dB
/// from 0 to 255 and is offset in such a way that -120 dBm corresponds to 
/// about 20."  In other words, it is linear: { 0 = -130dBm, 255 = -2.5dBm }.
/// This function turns the coded value into a normal, signed int.

/// @note In future implementations there may be a selection to allow OpenTag
///       to use half-dBm precision in all areas of the system, but for now it
///       uses whole-dBm (1dBm) precision.  Most antennas have >1dBm variance,
///       (often >2dBm) so unless you have an advanced subsampling algorithm 
///       for range-finding -- probably requiring a kalman filter in the
///       location engine -- using half-dBm will not yield an improvement.

    ot_int rssi_val;
    rssi_val    = (ot_int)encoded_value;    // Convert to signed int
    //rssi_val  >>= 1;                        // Make whole-dBm (divide by 2)
    //rssi_val   -= 130;                      // Apply 130 dBm offset
    rssi_val   -= 260;                      // Apply 130 dBm offset
    return rssi_val;
}


ot_u8 spirit1_calc_rssithr(ot_u8 input) {
/// SPIRIT1 treats RSSI thresholding through the normal RSSI engine.  The specs
/// are the same as those used in spirit1_calc_rssi() above, but the process is
/// using different input and output.
/// 
/// Input is a whole-dBm value encoded linearly as: {0=-140dBm, 127=-13dBm}.
/// Output is the value that should go into SPIRIT1 RSSI_TH field.
    ot_int rssi_thr;
    
    // SPIRIT1 uses -130 as baseline, DASH7 -140
    // Clip baseline at 0
    rssi_thr = (ot_int)input - 10;
    if (rssi_thr < 0)                               
        rssi_thr = 0;  
        
    // Multiply by 2 to yield half-dBm.
    rssi_thr  <<= 1;                                
    return rssi_thr;
}

    

void spirit1_set_txpwr(ot_u8* pwr_code) {
/// Sets the tx output power.
/// "pwr_code" is a value, 0-127, that is: eirp_code/2 - 40 = TX dBm
/// i.e. eirp_code=0 => -40 dBm, eirp_code=80 => 0 dBm, etc
    static const ot_u8 pa_lut[84] = {
        0x57, 0x57, 0x56, 0x55, 0x54, 0x53, 0x53, 0x52, 0x52, 0x50,     //-30 to -25.5
        0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4B, 0x4A, 0x49, 0x48, 0x47,     //-25 to -20.5
        0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40, 0x3F, 0x3E, 0x3C,     //-20 to -15.5
        0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x34, 0x33, 0x32, 0x31,     //-15 to -10.5
        0x30, 0x2F, 0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x27, 0x26, 0x25,     //-10 to -5.5
        0x24, 0x23, 0x22, 0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x19,     //-5 to -0.5
        0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10, 0x0F,     // 0 to 4.5
        0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05,     // 5 to 9.5
        0x04, 0x03, 0x02, 0x01                                          // 10 to 11.5
    };
    
    ot_u8   pa_table[10];
    ot_u8*  cursor;
    ot_int  step;
    ot_int  eirp_val;
    
    ///@todo autoscaling algorithm, and refresh value in *pwr_code
    // Autoscaling: Try to make RSSI at receiver be -90 < RX_RSSI < -80 
    // The simple algorithm uses the last_rssi and last_linkloss values
    //if (*pwr_code & 0x80) {
    //}
    //else {
    //}
    
    // Not autoscaling: extract TX power directly from pwr_code
    eirp_val = *pwr_code;
    
    // Offset SPIRIT1 PA CFG to match DASH7 PA CFG, plus antenna losses 
    // SPIRIT1: 0 --> -30 dBm, 83 --> 11.5 dBm, half-dBm steps
    // DASH7: 0 --> -40 dBm, 127 --> 23.5 dBm, half-dBm steps
    eirp_val += (-10*2) + (RF_HDB_ATTEN);
    
    // Adjust base power code in case it is out-of-range:
    // SPIRIT1 PA starts at -30, DASH7 pwr_code starts at -40.
    if (eirp_val < 0)       eirp_val = 0;
    else if (eirp_val > 83) eirp_val = 83;

    // Build PA RAMP using 8 steps of variable size.
    pa_table[0] = 0;
    pa_table[1] = RFREG(PAPOWER8);
    cursor      = &pa_table[2];
    step        = eirp_val >> 3;
    do {
        *cursor++   = pa_lut[eirp_val];
        eirp_val   -= step;
    } while (cursor != &pa_table[9]);

    
    // Write new PA Table to device
    spirit1_spibus_io(10, 0, pa_table);
}


ot_bool spirit1_check_cspin(void) {
    return (ot_bool)(RADIO_IRQ2_PORT->IDR & RADIO_IRQ2_PIN);
}






/** Common GPIO setup & interrupt functions  <BR>
  * ========================================================================<BR>
  * Your radio ISR function should be of the type void radio_isr(ot_u8), as it 
  * will be a soft ISR.  The input parameter is an interrupt vector.  The vector
  * values are shown below:
  *
  * -------------- RX MODES (set spirit1_iocfg_rx()) --------------
  * IMode = 0       RX Timeout (Finished):      0 
  * (Listen)        Sync word RX'ed:            1 
  *                 RX FIFO thr [IRQ off]:      -
  *                 
  * IMode = 2       RX Finished:                2  
  * (RX Data)       RX discarded (un-sync):     3
  *                 RX FIFO threshold:          4
  *
  * -------------- TX MODES (set spirit1_iocfg_tx()) --------------
  * IMode = 5       CCA Sense Timeout:          5   (pass)
  * (CSMA)          CS Indicator:               6   (fail)
  *                 TX FIFO thr [IRQ off]:      - 
  *
  * IMode = 7       TX finished:                7
  * (TX)            CS Indicator [IRQ off]:     -
  *                 TX FIFO threshold:          9 
  */

void platform_isr_rtcwakeup() {
/// Decrement dll counter... that's it
    dll.counter--;
}

void spirit1_start_counter() {
    RTC->CR |= RTC_CR_WUTE;
}

void spirit1_stop_counter() {
    //dll.counter = 0;          // not necessary or helpful
    RTC->CR &= ~RTC_CR_WUTE;
}

ot_u16 spirit1_get_counter() {
    return dll.counter;
}


/// Simple configuration method: 
/// This I/O configuration does not use many of the SPIRIT1 advanced features.
/// Those features will be experimented-with in the future.

///@todo test packet-discarding method, see if nIRQ must be added for RX

static const ot_u8 gpio_rx[5] = { 
    0, RFREG(GPIO2_CONF),
    RFGPO(RX_FIFO_ALMOST_FULL),  //indicate buffer threshold condition (kept for RX)
    RFGPO(SYNC_WORD),            //indicate when sync word is qualified
    RFGPO(TRX_INDICATOR)             //indicate when RX is active (falling edge)
};

static const ot_u8 gpio_tx[5] = { 
    0, RFREG(GPIO2_CONF),
    RFGPO(TX_FIFO_ALMOST_EMPTY), //indicate buffer threshold condition
    RFGPO(RSSI_ABOVE_THR),       //indicate if RSSI goes above/below CCA threshold
    RFGPO(TRX_INDICATOR)         //indicate when TX or RX is active
};
    
inline void spirit1_iocfg_rx()  {
/// All EXTIs for RX and TX are rising-edge detect, so the edge-select bit is
/// set universally following chip startup.
    EXTI->PR = RFI_ALL;   //clear all pending bits
    spirit1_spibus_io(5, 0, (ot_u8*)gpio_rx);
}

inline void spirit1_iocfg_tx()  {
/// All EXTIs for RX and TX are rising-edge detect, so the edge-select bit is
/// set universally following chip startup.
    EXTI->PR = RFI_ALL;   //clear all pending bits
    spirit1_spibus_io(5, 0, (ot_u8*)gpio_tx);
}


void sub_int_config(ot_u32 ie_sel) {
    ot_u32 scratch;
    EXTI->PR    = (ot_u32)RFI_ALL;
    scratch     = EXTI->IMR;
    scratch    &= ~((ot_u32)RFI_ALL);
    scratch    |= ie_sel;
    EXTI->IMR   = scratch;
}

void spirit1_int_off()      {   sub_int_config(0);   }

void spirit1_int_listen()   {   spirit1.imode = MODE_Listen;    
                                sub_int_config(RFI_LISTEN);     }

void spirit1_int_rxdata()   {   spirit1.imode = MODE_RXData;
                                sub_int_config(RFI_RXDATA);   }

void spirit1_int_csma()     {   spirit1.imode = MODE_CSMA;
                                sub_int_config(RFI_CSMA);     }
                                
void spirit1_int_txdata()   {   spirit1.imode = MODE_TXData;
                                sub_int_config(RFI_TXFIFO);   }
                                
void spirit1_int_txdone() {
    ot_u32 scratch;
    scratch     = EXTI->IMR & ~RFI_TXFIFO;
    scratch    |= RFI_TXEND;
    EXTI->PR    = RFI_TXEND;
    EXTI->IMR   = scratch;
}


void spirit1_int_force(ot_u16 ifg_sel)   { EXTI->SWIER |= (ot_u32)ifg_sel; }
void spirit1_int_turnon(ot_u16 ie_sel)   { EXTI->IMR   |= (ot_u32)ie_sel;  }

void spirit1_int_turnoff(ot_u16 ie_sel)  { 
    EXTI->PR    = (ot_u32)ie_sel;
    EXTI->IMR  &= ~((ot_u32)ie_sel); 
}



void spirit1_irq0_isr() {
    spirit1_virtual_isr(spirit1.imode);
}

void spirit1_irq1_isr() {
    spirit1_virtual_isr(spirit1.imode + 1);
}

void spirit1_irq2_isr() {
    spirit1_virtual_isr(spirit1.imode + 2);
}

void spirit1_irq3_isr() {
    spirit1_virtual_isr(spirit1.imode + 3);
}




#endif //#if from top of file

