/* Copyright 2013-2014 JP Norair
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
  * @file       /board/stm32l0xx/charon_r0.h
  * @author     JP Norair
  * @version    R100
  * @date       29 Sept 2014
  * @brief      Board Configuration for Charon R0
  * @ingroup    Platform
  *
  * Do not include this file, include OT_platform.h
  *
  * SPECIAL NOTE: search for "note BOARD Macro" lines in this file.  These
  * must be ported to other STM32L boards, because the OpenTag STM32L platform 
  * implementation depends on them.
  *
  ******************************************************************************
  */
  

#ifndef __charon_r0_H
#define __charon_r0_H

/// MCU definition for the board must be the first thing
#define __STM32L052K8__

#include <app/app_config.h>
#include <platform/hw/STM32L0xx_config.h>
#include <platform/interrupts.h>




/// Macro settings: ENABLED, DISABLED, NOT_AVAILABLE
#ifdef ENABLED
#   undef ENABLED
#endif
#define ENABLED  1

#ifdef DISABLED
#   undef DISABLED
#endif
#define DISABLED  0

#ifdef NOT_AVAILABLE
#   undef NOT_AVAILABLE
#endif
#define NOT_AVAILABLE   DISABLED





/** MCU Configuration settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the STM32L variants on this board/build
  *
  * On this board, the Jupiter DK, you can use USB or UART for MPipe
  */
#define MCU_CONFIG(VAL)                 MCU_CONFIG_##VAL   // FEATURE 
#define MCU_CONFIG_MULTISPEED           DISABLED                            // Allows usage of MF-HF clock boosting
#define MCU_CONFIG_MAPEEPROM            DISABLED
#define MCU_CONFIG_MPIPECDC             ENABLED                             // USB-CDC MPipe implementation
#define MCU_CONFIG_MPIPEUART            (MCU_CONFIG_MPIPECDC!=ENABLED)      // UART MPipe Implementation
#define MCU_CONFIG_MPIPEI2C             DISABLED                            // I2C MPipe Implementation
#define MCU_CONFIG_MEMCPYDMA            ENABLED                             // MEMCPY DMA should be lower priority than MPIPE DMA
#define MCU_CONFIG_USB                  (MCU_CONFIG_MPIPECDC == ENABLED)
#define MCU_CONFIG_VOLTLEVEL            2  // 3=1.2, 2=1.5V, 1=1.8V





/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.  For this configuration, 
  * Veelite is put into FLASH.
  *
  * The STM32L0 uses 128 byte Flash pages and 2KB Flash sectors.  Therefore, it
  * is best to allocate the FS in Flash on 2KB boundaries because this is the 
  * resolution that can be write protected (or, as with FS, *NOT* write 
  * protected).  Best practice with STM32 chips is to put the FS at the back of
  * of the Flash space because this seems to work best with the debugger HW.
  */

// You can comment-out these if you want to use chip defaults
#ifndef SRAM_AVAILABLE
#   define SRAM_AVAILABLE            (8*1024)
#endif
#ifndef EEPROM_AVAILABLE
#   define EEPROM_AVAILABLE          (2*1024)
#endif
#ifndef FLASH_AVAILABLE
#   define FLASH_AVAILABLE      (64*1024)
#endif


// Using EEPROM: Pages figure is irrelevant
#define FLASH_NUM_PAGES         (FLASH_AVAILABLE/FLASH_PAGE_SIZE)
#define FLASH_FS_ADDR           (EEPROM_START_ADDR)
#define FLASH_FS_PAGES          0
#define FLASH_FS_FALLOWS        0 
#define FLASH_FS_ALLOC          (EEPROM_AVAILABLE) 








/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  * 1. Two buttons are supported, but only if MPIPE is not enabled.
  *    <LI> SW1: multiplexed with TX/SCL </LI>
  *    <LI> SW2: multiplexed with RX/SDA </LI>
  * 
  * 2. Two LEDs can be supported.
  *    <LI> TRIG1 (typically Orange LED) is multiplexed on TEST pin.  TEST pin
  *         is a feature of the HayTag, and it has no special purpose in HW. </LI>
  *    <LI> TRIG2 (typically Green LED) is multiplexed on the TRACE-SWO pin.
  *         If you are using an SWO Tracer, you would need to disable TRIG2.
  *         (editorial note: I have never used or even seen an SWO Tracer) </LI>
  *
  * 3. MPIPE can support UART TX/RX (no flow control) or I2C.  In either case,
  *    a break character can be transmitted by the client ahead of the packet 
  *    in order to wakeup the system.  Assume that 20us is needed between the
  *    client-TX falling edge and start-bit rising edge.
  *
  * 4. The Debug and Upload interface is SWD.  There is no serial bootloader
  *    capability.  (Editorial Note: A ST-Link-V2 has SWD and a low price of
  *    US$25.  It can work with OpenOCD, so the toolchain is still very cheap.)
  * 
  * 5. The SPIRIT1 SPI benefits from the highest speed clock up to 20 MHz.
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_PARAM(VAL)                BOARD_PARAM_##VAL

#define BOARD_FEATURE_MPIPE             ENABLED
#define BOARD_FEATURE_USBCONVERTER      BOARD_FEATURE_MPIPE         // Is UART connected via USB converter?

// MPIPE UART modes are deprecated.  Only Break-mode should be used.
#define BOARD_FEATURE_MPIPE_DIRECT      DISABLED                    // BOARD_FEATURE_MPIPE
#define BOARD_FEATURE_MPIPE_BREAK       BOARD_FEATURE_MPIPE         // Send/receive leading break for wakeup
#define BOARD_FEATURE_MPIPE_CS          DISABLED                    // Chip-Select / DTR wakeup control
#define BOARD_FEATURE_MPIPE_FLOWCTL     DISABLED                    // RTS/CTS style flow control 

#define BOARD_FEATURE_LFXTAL            DISABLED                                // LF XTAL used as Clock source
#define BOARD_FEATURE_HFXTAL            DISABLED                                // HF XTAL used as Clock source
#define BOARD_FEATURE_HFBYPASS          DISABLED                                // Use an externally driven oscillator
#define BOARD_FEATURE_HFCRS             (BOARD_FEATURE_HFXTAL != ENABLED)       // Use STM32L0's Clock-Recovery-System for USB
#define BOARD_FEATURE_RFXTAL            DISABLED                                // XTAL for RF chipset
#define BOARD_FEATURE_RFXTALOUT         DISABLED
#define BOARD_FEATURE_PLL               (MCU_CONFIG_MULTISPEED == ENABLED)
#define BOARD_FEATURE_STDSPEED          (MCU_CONFIG_MULTISPEED == ENABLED)
#define BOARD_FEATURE_FULLSPEED         ENABLED
#define BOARD_FEATURE_FULLXTAL          DISABLED
#define BOARD_FEATURE_FLANKSPEED        (MCU_CONFIG_MULTISPEED == ENABLED)
#define BOARD_FEATURE_FLANKXTAL         (BOARD_FEATURE_HFCRS != ENABLED)
#if (MCU_CONFIG(USB) && BOARD_FEATURE(PLL) && (BOARD_FEATURE(HFXTAL) || BOARD_FEATURE(HFBYPASS)))
#   warning "Charon board does not normally support XTAL-based USB."
#   define BOARD_FEATURE_USBPLL         ENABLED
#elif MCU_CONFIG(USB)
#   undef BOARD_FEATURE_HFCRS
#   define BOARD_FEATURE_HFCRS          ENABLED
#   define BOARD_FEATURE_USBPLL         DISABLED
#endif

#define BOARD_FEATURE_INVERT_TRIG1      DISABLED
#define BOARD_FEATURE_INVERT_TRIG2      DISABLED

#define BOARD_PARAM_TRIGS               2
#define BOARD_PARAM_LFHz                32768
#define BOARD_PARAM_LFtol               0.00002
#define BOARD_PARAM_MFHz                4200000
#define BOARD_PARAM_MFmult              1                       // Main CLK = HFHz * (MFmult/MFdiv)
#define BOARD_PARAM_MFdiv               1
#define BOARD_PARAM_MFtol               0.02
#define BOARD_PARAM_HFHz                16000000
#define BOARD_PARAM_HFtol               0.02
#define BOARD_PARAM_HFppm               20000
//#define BOARD_PARAM_RFHz                48000000
//#define BOARD_PARAM_RFdiv               6
//#define BOARD_PARAM_RFout               (BOARD_PARAM_RFHz/BOARD_PARAM_RFdiv)
//#define BOARD_PARAM_RFtol               0.00003
#define BOARD_PARAM_PLLout              96000000
#define BOARD_PARAM_PLLmult             (BOARD_PARAM_PLLout/BOARD_PARAM_HFHz)
#define BOARD_PARAM_PLLdiv              3
#define BOARD_PARAM_PLLHz               (BOARD_PARAM_PLLout/BOARD_PARAM_PLLdiv)

// These are defined at FULL SPEED
#define BOARD_PARAM_AHBCLKDIV           1                       // AHB Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB2CLKDIV          1                       // APB2 Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB1CLKDIV          1                       // APB1 Clk = Main CLK / AHBCLKDIV

// SWD Interface: Nucleo used SWD in all cases, never JTAG.
// The SWO (Trace) pin is available, but not often used.
#define BOARD_SWDIO_PORT                GPIOA
#define BOARD_SWDIO_PINNUM              13
#define BOARD_SWDIO_PIN                 (1<<13)
#define BOARD_SWDCLK_PORT               GPIOA
#define BOARD_SWDCLK_PINNUM             14
#define BOARD_SWDCLK_PIN                (1<<14)
#define BOARD_TRACESWO_PORTNUM          1
#define BOARD_TRACESWO_PORT             GPIOB
#define BOARD_TRACESWO_PINNUM           (1<<3)
#define BOARD_TRACESWO_PIN              3


// ADC Analog inputs on PA0,PA1,PA4,PB0,PC1,PC0 or "A0,A1,A2,A3,A4,A5"
#define BOARD_ADC_PINS                  1
#define BOARD_ADC_PORTS                 1
#define BOARD_ADC_0_PORTNUM             0
#define BOARD_ADC_0_PORT                GPIOA
#define BOARD_ADC_0_CHAN                1
#define BOARD_ADC_0_PINNUM              1
#define BOARD_ADC_0_PIN                 (1<<BOARD_ADC_0PINNUM)
#define BOARD_ADC_0E                    ENABLED
#define BOARD_ADC_0E_PORTNUM            0
#define BOARD_ADC_0E_PORT               GPIOA
#define BOARD_ADC_0E_PINNUM             2
#define BOARD_ADC_0E_PIN                (1<<BOARD_ADC_0E_PINNUM)
#define BOARD_ADC_0E_OTYPE              (GPIO_OTYPE_OPENDRAIN)      //OPEN DRAIN
#define BOARD_ADC_0E_POLARITY           0                           //ACTIVE LOW
#define BOARD_ADC_0_ENABLE()            (BOARD_ADC_0E_PORT->BSRR = (BOARD_ADC_0E_PIN<<16))
#define BOARD_ADC_0_DISABLE()           (BOARD_ADC_0E_PORT->BSRR = BOARD_ADC_0E_PIN)


// SW1 is on PB8, which is also the IFRES pin used to reset the I/O Interface.
// IFRES is active-low with a pullup.
//#define BOARD_SW1_PORTNUM               1
//#define BOARD_SW1_PORT                  GPIOB
//#define BOARD_SW1_PINNUM                8
//#define BOARD_SW1_PIN                   (BOARD_SW1_PINNUM<<3)
//#define BOARD_SW1_POLARITY              0
//#define BOARD_SW1_PULLING               1
//#define BOARD_IFRES_PORTNUM             BOARD_SW1_PORTNUM
//#define BOARD_IFRES_PORT                BOARD_SW1_PORT
//#define BOARD_IFRES_PINNUM              BOARD_SW1_PINNUM
//#define BOARD_IFRES_PIN                 BOARD_SW1_PIN
//#define BOARD_IFRES_POLARITY            BOARD_SW1_POLARITY
//#define BOARD_IFRES_PULLING             BOARD_SW1_PULLING


// SPI Interface on PB3,4,5 and PA15
#define BOARD_SPI_ID                    1        //SPI1
#define BOARD_SPI_PORTNUM               1        //Port B
#define BOARD_SPI_PORT                  GPIOB    
#define BOARD_SPI_MOSIPINNUM            5
#define BOARD_SPI_MISOPINNUM            4
#define BOARD_SPI_SCLKPINNUM            3
#define BOARD_SPI_CSNPORTNUM            0        //Port A
#define BOARD_SPI_CSNPORT               GPIOA
#define BOARD_SPI_CSNPINNUM             15
#define BOARD_SPI_MOSIPIN               (1<<BOARD_SPI_MOSIPINNUM)
#define BOARD_SPI_MISOPIN               (1<<BOARD_SPI_MISOPINNUM)
#define BOARD_SPI_SCLKPIN               (1<<BOARD_SPI_SCLKPINNUM)
#define BOARD_SPI_CSNPIN                (1<<BOARD_SPI_CSNPINNUM)


// TEST0, TEST1 pins are shared with SPI
// They are also connected to the LEDs
#define BOARD_TEST0_PORTNUM             BOARD_SPI_PORTNUM 
#define BOARD_TEST0_PORT                BOARD_SPI_PORT
#define BOARD_TEST0_PINNUM              BOARD_SPI_MOSIPINNUM
#define BOARD_TEST0_PIN                 BOARD_SPI_MOSIPIN
#define BOARD_TEST0_POLARITY            1
#define BOARD_TEST0_PULLING             0
#define BOARD_TEST1_PORTNUM             BOARD_SPI_CSNPORTNUM
#define BOARD_TEST1_PORT                BOARD_SPI_CSNPORT
#define BOARD_TEST1_PINNUM              BOARD_SPI_CSNPINNUM
#define BOARD_TEST1_PIN                 BOARD_SPI_CSNPIN
#define BOARD_TEST1_POLARITY            1
#define BOARD_TEST1_PULLING             0
#define BOARD_LEDG_PORTNUM              BOARD_TEST0_PORTNUM
#define BOARD_LEDG_PORT                 BOARD_TEST0_PORT
#define BOARD_LEDG_PINNUM               BOARD_TEST0_PINNUM
#define BOARD_LEDG_PIN                  BOARD_TEST0_PIN
#define BOARD_LEDG_POLARITY             BOARD_TEST0_POLARITY
#define BOARD_LEDO_PORTNUM              BOARD_TEST1_PORTNUM
#define BOARD_LEDO_PORT                 BOARD_TEST1_PORT
#define BOARD_LEDO_PINNUM               BOARD_TEST1_PINNUM
#define BOARD_LEDO_PIN                  BOARD_TEST1_PIN
#define BOARD_LEDO_POLARITY             BOARD_TEST1_POLARITY



// Main serial connections, used at least by MPIPE

// I2C on PB6/PB7 
//#define BOARD_I2C_PORTNUM               1                       
//#define BOARD_I2C_PORT                  GPIOB
//#define BOARD_I2C_ID                    1
//#define BOARD_I2C_SCLPINNUM             6
//#define BOARD_I2C_SDAPINNUM             7
//#define BOARD_I2C_SCLPIN                (1<<BOARD_I2C_SCLPINNUM)
//#define BOARD_I2C_SDAPIN                (1<<BOARD_I2C_SDAPINNUM)

// UART on PB6/PB7
//#define BOARD_UART_PORTNUM          1
//#define BOARD_UART_PORT             GPIOB
//#define BOARD_UART_ID               1
//#define BOARD_UART_TXPINNUM         6
//#define BOARD_UART_RXPINNUM         7
//#define BOARD_UART_TXPIN            (1<<BOARD_UART_TXPINNUM)
//#define BOARD_UART_RXPIN            (1<<BOARD_UART_RXPINNUM)

// USB is always on PA11/A12
#define BOARD_USB_PORTNUM               0
#define BOARD_USB_PORT                  GPIOA
#define BOARD_USB_DMPINNUM              11
#define BOARD_USB_DPPINNUM              12
#define BOARD_USB_DMPIN                 (1<<BOARD_USB_DMPINNUM)
#define BOARD_USB_DPPIN                 (1<<BOARD_USB_DPPINNUM)




/** BOARD Macros (most of them, anyway) <BR>
  * ========================================================================<BR>
  * BOARD Macros are required by the OpenTag STM32L platform implementation.
  * We can assume certain things at the platform level, but not everything,
  * which is why BOARD Macros are required.
  *
  * If your board is implementing a radio (extremely likely), there are likely
  * some additional Board macros down near the radio configuration section.
  */


// AES Engine is only on STM32L06x
#define _CRYPCLK_N      0   //RCC_AHBENR_CRYPEN
#define _CRYPCLK_LP     0   //RCC_AHBSMENR_CRYPSMEN

// Random Number Generator is used by OpenTag, but only on-demand.  The driver
// must manage the clock bit
#define _RNGCLK_N       RCC_AHBENR_RNGEN
#define _RNGCLK_LP      RCC_AHBSMENR_RNGSMEN

// Touchscreen controller is not used by OpenTag, so the driver must manage it
// on-demand
#define _TOUCHCLK_N     0   //RCC_AHBENR_TOUCHEN
#define _TOUCHCLK_LP    RCC_AHBSMENR_TSCSMEN

//The built-in CRC engine is used by Opentag for many things, but never in sleep
#define _CRCCLK_N        RCC_AHBENR_CRCEN
#define _CRCCLK_LP       0       //RCC_AHBSMENR_CRCSMEN

// SRAM should always be clocked during SLEEP
#define _SRAMCLK_LP      RCC_AHBLPENR_SRAMLPEN

// Flash should be enabled while awake and in sleep unless you seriously modify 
// OpenTag to run exclusively from RAM
#define _MIFCLK_N       RCC_AHBENR_MIFEN
#define _MIFCLK_LP      RCC_AHBSMENR_MIFSMEN

// DMA is used in MEMCPY and thus it could be needed at any time during active
// runtime.  So, the active-clock must be enabled permanently.
#if (MCU_CONFIG_MEMCPYDMA)
#   define _DMACLK_N    RCC_AHBENR_DMA1EN
#   define _DMACLK_LP   RCC_AHBSMENR_DMA1SMEN
#else
#   define _DMACLK_N    0
#   define _DMACLK_LP   RCC_AHBSMENR_DMA1SMEN
#   define _DMACLK_DYNAMIC
#endif

// This board enabled all GPIO on ports A, B, C, and D.
// On startup (_SU), port H is also enabled
#define _GPIOCLK_N       (RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN)
#define _GPIOCLK_SU      (_GPIOCLK_N | RCC_IOPENR_GPIOCEN)
#define _GPIOCLK_LP      (_GPIOCLK_N)


#define _IOPENR_STARTUP  (_DMACLK_N | _MIFCLK_N | _CRCCLK_N | _GPIOCLK_SU)
#define _IOPENR_RUNTIME  (_CRYPCLK_N | _MIFCLK_N | _CRCCLK_N | _GPIOCLK_N)


//@note BOARD Macro for Peripheral Clock initialization at startup
static inline void BOARD_PERIPH_INIT(void) {
    // 1. AHB Clock Setup for Active Mode & Sleep
    RCC->AHBENR     = (_CRYPCLK_N | _RNGCLK_N | _TOUCHCLK_N | _CRCCLK_N | _MIFCLK_N | _DMACLK_N);
    RCC->AHBSMENR   = (_CRYPCLK_LP| _RNGCLK_LP| _TOUCHCLK_LP| _CRCCLK_LP| _MIFCLK_LP| _DMACLK_LP);

    // 2. I/O Clock Setup for Sleep and Startup
    RCC->IOPENR     = _IOPENR_STARTUP;

    // 3. APB2 Clocks in Active Mode.  APB2 is the high-speed peripheral bus.  
    // The default is all-off, and it is the job of the peripheral drivers to 
    // enable/disable their clocks as needed.  SYSCFG and Debug Module are on
    // without needing driver support
#   if defined(__DEBUG__) || defined(__PROTO__)
    RCC->APB2ENR   = (RCC_APB2ENR_DBGMCUEN | RCC_APB2ENR_SYSCFGEN);
#   else
    RCC->APB2ENR   = (RCC_APB2ENR_SYSCFGEN);
#   endif

    // 3. APB1 Clocks in Active Mode.  APB1 is the low-speed peripheral bus.
    // The default is all-off, and it is the job of the peripheral drivers to 
    // enable/disable their clocks as needed.  The exceptions are LPTIM1 and 
    // PWR, which are fundamental to OpenTag.
    RCC->APB1ENR   = (RCC_APB1ENR_LPTIM1EN | RCC_APB1ENR_PWREN); 
}



//@note BOARD Macro for DMA peripheral enabling
static inline void BOARD_DMA_CLKON(void) {
#ifdef _DMACLK_DYNAMIC
    RCC->AHBENR    |= RCC_AHBENR_DMA1EN;
#endif
}

static inline void BOARD_DMA_CLKOFF(void) {
#ifdef _DMACLK_DYNAMIC
    RCC->AHBENR    &= ~RCC_AHBENR_DMA1EN;
#endif
}



///@note BOARD Macro for EXTI initialization.  See also EXTI macros at the
///      bottom of the page.
static inline void BOARD_EXTI_STARTUP(void) {
    // EXTI0-3: A0, A1, B2, UART-RX Break
    SYSCFG->EXTICR[0]   = 0;
    
    // EXTI4-7: B4, B5, B6, C7
    SYSCFG->EXTICR[1]   = 0;
    
    // EXTI8-11: RFGPIO1 A8, A9, A10, A11
    SYSCFG->EXTICR[2]   = 0;
    
    // EXTI12-15: A12, B13, B14, A15
    SYSCFG->EXTICR[3]   = 0;
}



///@note BOARD Macro for initializing GPIO ports at startup, pursuant to the
///      connections in the schematic of this board.  This funciotn
static inline void BOARD_PORT_STARTUP(void) {  
    /// Initialize ports/pins exclusively used within this platform module.
    /// A. Trigger Pins
    /// B. Random Number ADC pins: A Zener can be used to generate noise.
    
    /// Configure Port A IO.  
    /// Port A is used for internal features and .
    // - A0 is unused, set to ANALOG (default)
    // - A1 is the ADC_0 input 
    // - A2 is the ADC_0E open drain output (active low, no-pullup)
    // - A3:10 are unused, set to ANALOG (default)
    // - A11:12 are USB I/O, which gets configured in the driver.  Set to HiZ
    // - A13:14 are SWD, set to ALT-DEBUG
    // - A15 is SPI-NSS and TEST1, set to push-pull output
#   ifdef __USE_SPI
    GPIOA->BSRR     = BOARD_SPI_CSNPIN;
#   endif

    //GPIOA->AFR[1]   = (0 << ((BOARD_USB_DMPINNUM-8)*4)) \
    //                | (0 << ((BOARD_USB_DPPINNUM-8)*4));

    GPIOA->MODER    = (GPIO_MODER_ANALOG    << (0*2)) \
                    | (GPIO_MODER_ANALOG    << (1*2)) \
                    | (GPIO_MODER_OUT       << (2*2)) \
                    | (GPIO_MODER_ANALOG    << (3*2)) \
                    | (GPIO_MODER_ANALOG    << (4*2)) \
                    | (GPIO_MODER_ANALOG    << (5*2)) \
                    | (GPIO_MODER_ANALOG    << (6*2)) \
                    | (GPIO_MODER_ANALOG    << (7*2)) \
                    | (GPIO_MODER_ANALOG    << (8*2)) \
                    | (GPIO_MODER_ANALOG    << (9*2)) \
                    | (GPIO_MODER_ANALOG    << (10*2)) \
                    | (GPIO_MODER_IN       << (11*2)) \
                    | (GPIO_MODER_IN       << (12*2)) \
                    | (GPIO_MODER_ALT       << (13*2)) \
                    | (GPIO_MODER_ALT       << (14*2)) \
                    | (GPIO_MODER_OUT       << (15*2));

    GPIOA->OTYPER   = (1<<2);
    
    GPIOA->OSPEEDR  = (GPIO_OSPEEDR_40MHz << (11*2)) \
                    | (GPIO_OSPEEDR_40MHz << (12*2)) \
                    | (GPIO_OSPEEDR_40MHz << (13*2)) \
                    | (GPIO_OSPEEDR_40MHz << (14*2));
    
    GPIOA->PUPDR    = 0;

    

    /* */
    /// Configure Port B IO.
    /// Port B is used for external (module) IO.
    // - B0:2 are unused and set to Analog mode
    // - B3:4 are SPI pins, which are managed by the driver.  Default=Analog
    // - B5 is used as a SPI-MOSI, but also TEST0.  Default=OUT
    // - B7 is unused and set to Analog mode
    // - B8:9 are setup for I2C bus: They start as output with Open Drain
    // - B10 is a PWM output, set to Analog until configured
    // - B11:15 are unused and set to Analog mode
    GPIOB->OTYPER   = (1 << (8)) | (1 << (9));
    GPIOB->BSRR     = (1 << (8)) | (1 << (9));

    GPIOB->MODER    = (GPIO_MODER_ANALOG << (0*2)) \
                    | (GPIO_MODER_ANALOG << (1*2)) \
                    | (GPIO_MODER_ANALOG << (2*2)) \
                    | (GPIO_MODER_ANALOG << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_OUT    << (5*2)) \
                    | (GPIO_MODER_ANALOG << (6*2)) \
                    | (GPIO_MODER_ANALOG << (7*2)) \
                    | (GPIO_MODER_ANALOG << (8*2)) \
                    | (GPIO_MODER_ANALOG << (9*2)) \
                    | (GPIO_MODER_ANALOG << (10*2)) \
                    | (GPIO_MODER_ANALOG << (11*2)) \
                    | (GPIO_MODER_ANALOG << (12*2)) \
                    | (GPIO_MODER_ANALOG << (13*2)) \
                    | (GPIO_MODER_ANALOG << (14*2)) \
                    | (GPIO_MODER_ANALOG << (15*2));
    
    GPIOB->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (3*2)) \
                    | (GPIO_OSPEEDR_10MHz << (4*2)) \
                    | (GPIO_OSPEEDR_10MHz << (5*2));
    
#   if defined(__USE_SPI)
    GPIOB->AFR[0]   = (0 << ((BOARD_SPI_MOSIPINNUM)*4)) \
                    | (0 << ((BOARD_SPI_MISOPINNUM)*4)) \
                    | (0 << ((BOARD_SPI_SCLKPINNUM)*4));
#   endif


    /// Configure Port C IO.
    /// Port C is unused
    
    // - C14:15 are 32kHz crystal driving, but there is no crystal
    GPIOC->MODER    = (GPIO_MODER_ANALOG << (14*2)) \
                    | (GPIO_MODER_ANALOG << (15*2));
    
    // Disable GPIOC and GPIOH, they are never altered after this.
    RCC->IOPENR = _IOPENR_RUNTIME;
}





static inline void BOARD_SPI_CLKON(void) {
#ifdef __USE_SPI
    BOARD_SPI_PORT->MODER &= ~((3 << (BOARD_SPI_SCLKPINNUM*2)) \
                            | (3 << (BOARD_SPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_SPI_MOSIPINNUM*2)) );
    
    BOARD_SPI_PORT->MODER |= (GPIO_MODER_ALT << (BOARD_SPI_SCLKPINNUM*2)) \
                            | (GPIO_MODER_ALT << (BOARD_SPI_MISOPINNUM*2)) \
                            | (GPIO_MODER_ALT << (BOARD_SPI_MOSIPINNUM*2));
    
  //BOARD_SPI_PORT->MODER ^= (3 << (BOARD_SPI_SCLKPINNUM*2)) \
                            | (2 << (BOARD_SPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_SPI_MOSIPINNUM*2));
#endif
}

static inline void BOARD_SPI_CLKOFF(void) {
#ifdef __USE_SPI
    BOARD_SPI_PORT->MODER &= ~((3 << (BOARD_SPI_SCLKPINNUM*2)) \
                            | (3 << (BOARD_SPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_SPI_MOSIPINNUM*2)) );
    
    BOARD_SPI_PORT->MODER |= (GPIO_MODER_OUT << (BOARD_SPI_SCLKPINNUM*2)) \
                            | (GPIO_MODER_IN << (BOARD_SPI_MISOPINNUM*2)) \
                            | (GPIO_MODER_OUT << (BOARD_SPI_MOSIPINNUM*2));
    
  //BOARD_SPI_PORT->MODER ^= (3 << (BOARD_SPI_SCLKPINNUM*2)) \
                            | (2 << (BOARD_SPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_SPI_MOSIPINNUM*2));
#endif
}



#include <platform/timers.h>    // for systim_stop_clocker()
static inline void BOARD_STOP(ot_int code) {
/// code comes from sys_sig_powerdown, but it is usually 0-3.
/// For all STM32L devices, 3 is full-idle and 2 is I/O-active-idle.  
/// Those are the only modes that should call this inline function.

    static const ot_u16 stop_flags[2] = {  
        (PWR_CR_LPSDSR | PWR_CR_CSBF), (PWR_CR_LPSDSR | PWR_CR_FWU | PWR_CR_ULP | PWR_CR_CSBF) };
        
    static const ot_u32 rcc_flags[2] = {
        (RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN), 0 };
        
    static const ot_u32 b_moder[2] = {
        0xFFFFFC0, 0xFFFFFFFF };
    
    ot_u16 scratch;
    
    code &= 1;
    
#   if defined(__RELEASE__)
      //GPIOA->MODER    = 0xFFFFFFFF;
      //GPIOB->MODER    = b_moder[code];
      //GPIOA->PUPDR    = 0;
      //GPIOB->PUPDR    = 0;      //can be ignored, always 0
      RCC->IOPENR    &= rcc_flags[code];
#   endif

    SysTick->CTRL = 0;
    SCB->SCR   |= SCB_SCR_SLEEPDEEP_Msk;
    scratch     = PWR->CR;
    scratch    &= ~(PWR_CR_DBP | PWR_CR_PDDS | PWR_CR_LPSDSR);
    scratch    |= stop_flags[code];
    PWR->CR     = scratch;
    
    EXTI->PR    = 0;
    systim_stop_clocker();
    platform_enable_interrupts();
    
    __WFI();
    
    // On Wakeup (from STOP) clear flags & re-enable backup register area
    PWR->CR |= (PWR_CR_DBP | PWR_CR_CWUF | PWR_CR_CSBF);
    
    // On wakeup, immediately reset SLEEPDEEP bit
    SCB->SCR &= ~((ot_u32)SCB_SCR_SLEEPDEEP_Msk);
    
    // Re-enable ports
#   if defined(__RELEASE__)
      RCC->IOPENR    |= _IOPENR_RUNTIME;
      //GPIOA->PUPDR    = GPIOA_PUPDR_DEFAULT;
      //GPIOB->PUPDR    = GPIOB_PUPDR_DEFAULT;    //can be ignored, always 0
      //GPIOA->MODER    = GPIOA_MODER_DEFAULT;
      //GPIOB->MODER    = GPIOB_MODER_DEFAULT;
#   endif
}




static inline void BOARD_PORT_STANDBY() {
///@todo implement this!
}




///@note BOARD Macro for initializing the STM32 PVD Module (voltage monitor) at
///      startup.  Only relevant for battery-powered boards.
static inline void BOARD_POWER_STARTUP(void) {
#   if (MCU_FEATURE_SVMONITOR == ENABLED)
    // Level0 is 1.9V, each level is +0.2V.  Check your battery spec and change
    // the _LEVx component to match it.  Typical Li-Ion undervoltage is 2.7V.
    PWR->CR    |= PWR_CR_PLS_LEV0 | PWR_CR_PVDE;  
#   endif
}



///@note BOARD Macro for initializing the STM32 Crystal startup routine, done
///      at startup.  It must startup ALL the crystals that you want to run 
///      during normal operation.
static inline void BOARD_XTAL_STARTUP(void) {
}



///@note BOARD Macros for turning-on and turning-off the High-speed XTAL.
///      High speed XTAL does not exist on this board, so it is empty.
static inline void BOARD_HSXTAL_ON(void) {
}
static inline void BOARD_HSXTAL_OFF(void) {
}



///@brief BOARD Macros for turning-on and turning-off the USB Clock, which may
///       be implemented in a lot of different ways depending on the board, the
///       MCU, and the configuration.
///
///@note  For STM32L0, the standard implementation is to use the CRS system in
///       combination with the PLL.
///

static inline void BOARD_USBCLK_ON(void) {
#if BOARD_FEATURE(USBPLL)
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    platform_ext_pllon();
#else
    platform_ext_usbcrson();
#endif
}

static inline void BOARD_USBCLK_OFF(void) {
#if BOARD_FEATURE(USBPLL)
    platform_ext_plloff();
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
#else
    platform_ext_usbcrsoff();
#endif
}

static inline void BOARD_USB_PORTENABLE(void) {
    USB->BCDR |= (ot_u32)USB_BCDR_DPPU;
    
    //BOARD_USB_PORT->MODER  |= (2 << (BOARD_USB_DMPINNUM*2)) \
                            | (2 << (BOARD_USB_DPPINNUM*2));
}

static inline void BOARD_USB_PORTDISABLE(void) {
    USB->BCDR &= ~(ot_u32)USB_BCDR_DPPU;
    
    //BOARD_USB_PORT->MODER  &= ~( (3 << (BOARD_USB_DMPINNUM*2)) \
                               | (3 << (BOARD_USB_DPPINNUM*2)) );
}

   


///@todo Create a more intelligent setup that knows how to use the UART, even
///      though for this board UART is not generally the MPipe
///@note BOARD Macros for Com module interrupt vectoring.  Connect these to
///      the Com interface driver you are using.  Check the schematic of your
///      board to see where the Com IRQ lines are routed. 
#if (MCU_CONFIG(MPIPEUART) && OT_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE_BREAK))
#   define _UART_RXSYNC_ISR()       mpipe_rxsync_isr()
#else
#   define _UART_RXSYNC_ISR()       ;
///@todo implement non-mpipe uart with break feature 
#endif

#define BOARD_COM_EXTI0_ISR();
#define BOARD_COM_EXTI1_ISR();
#define BOARD_COM_EXTI2_ISR();
#define BOARD_COM_EXTI3_ISR()   _UART_RXSYNC_ISR()
#define BOARD_COM_EXTI4_ISR();
#define BOARD_COM_EXTI5_ISR();
#define BOARD_COM_EXTI6_ISR();     
#define BOARD_COM_EXTI7_ISR();     
#define BOARD_COM_EXTI8_ISR();     
#define BOARD_COM_EXTI9_ISR();
#define BOARD_COM_EXTI10_ISR();   
#define BOARD_COM_EXTI11_ISR();
#define BOARD_COM_EXTI12_ISR();
#define BOARD_COM_EXTI13_ISR();
#define BOARD_COM_EXTI14_ISR();
#define BOARD_COM_EXTI15_ISR();





/** Note: Clocking for the Board's MCU      <BR>
  * ========================================================================<BR>
  * The STM32L can be clocked up to 32MHz, but generally we use it at 16 MHz.
  * At the 1.5V core setting, 16MHz can be used with 1 flash waitstate, but 
  * OpenTag code tends to abhor branching, so flash wait states have less 
  * affect on OT efficiency than they have on most other codebases.
  */
#define MCU_CONFIG_LFXTALHz          BOARD_PARAM_LFHz
#define MCU_CONFIG_LFXTALtol         BOARD_PARAM_LFtol
//#define MCU_CONFIG_LFOSCHz           BOARD_PARAM_LFHz
//#define MCU_CONFIG_LFOSCtol          BOARD_PARAM_LFtol
//#define MCU_CONFIG_XTALHz            BOARD_PARAM_HFHz
//#define MCU_CONFIG_XTALmult          BOARD_PARAM_HFmult
//#define MCU_CONFIG_XTALtol           BOARD_PARAM_HFtol
#define MCU_CONFIG_OSCHz             BOARD_PARAM_HFHz
#define MCU_CONFIG_OSCmult           BOARD_PARAM_HFmult
#define MCU_CONFIG_OSCtol            BOARD_PARAM_HFtol

#define PLATFORM_MCLK_DIV           BOARD_PARAM_MCLKDIV
#define PLATFORM_SMCLK_DIV          BOARD_PARAM_SMCLKDIV

#define PLATFORM_LSCLOCK_PINS       BOARD_LFXT_PINS
#define PLATFORM_LSCLOCK_CONF       BOARD_LFXT_CONF
#define PLATFORM_HSCLOCK_PINS       BOARD_HFXT_PINS
#define PLATFORM_HSCLOCK_CONF       BOARD_HFXT_CONF
#define PLATFORM_LSCLOCK_HZ         BOARD_PARAM_LFHz
#define PLATFORM_LSCLOCK_ERROR      BOARD_PARAM_LFtol
#define PLATFORM_MSCLOCK_HZ         (BOARD_PARAM_MFHz)
#define PLATFORM_MSCLOCK_ERROR      BOARD_PARAM_MFtol
#define PLATFORM_HSCLOCK_HZ         BOARD_PARAM_HFHz
#define PLATFORM_HSCLOCK_ERROR      BOARD_PARAM_HFtol
#define PLATFORM_PLLCLOCK_OUT       ((BOARD_PARAM_HFHz/BOARD_PARAM_HFdiv)*BOARD_PARAM_PLLmult)
#define PLATFORM_PLLCLOCK_HZ        (PLATFORM_PLLCLOCK_OUT/BOARD_PARAM_PLLdiv)
#define PLATFORM_PLLCLOCK_ERROR     BOARD_PARAM_RFtol




/** Peripheral definitions for this platform <BR>
  * ========================================================================<BR>
  * <LI> OT_GPTIM:  "General Purpose Timer" used by OpenTag kernel and other
  *                   internal stuff (sometimes Radio MAC timer). </LI>
  * <LI> OT_TRIG:   Optional test trigger(s) usable in OpenTag apps.  Often the 
  *                   triggers are implemented on LED pins.  Trig1 is usually
  *                   green (typ 570nm) and Trig2 orange (typ 605nm). </LI>
  * <LI> OT_SWITCH: Optional input switches/sources.  Usually there is at least
  *                   one.  Many apps (especially demos) require a button. </LI>
  * <LI> OT_GWNADC: Optional floating pin or other type of noise source that 
  *                   can be used with ADC-based true-random-number generator </LI>
  * <LI> OT_GWNDRV: Optional driver pin to power some sort of noise source </LI>
  * <LI> MPIPE:     The wireline interface, usually either UART or USB-CDC, but 
  *                   possible to use with a multi-master I2C or SPI as well </LI>
  */
#define OT_GPTIM_ID         'L'
#define OT_GPTIM            LPTIM1
#define OT_GPTIM_CLOCK      32768
#define OT_GPTIM_RES        1024
#define OT_GPTIM_SHIFT      0
#define TI_TO_CLK(VAL)      ((OT_GPTIM_RES/1024)*VAL)
#define CLK_TO_TI(VAL)      (VAL/(OT_GPTIM_RES/1024))


#if (OT_GPTIM_CLOCK == BOARD_PARAM_LFHz)
#   define OT_GPTIM_ERROR   BOARD_PARAM_LFtol
#else
#   define OT_GPTIM_ERROR   BOARD_PARAM_HFtol
#endif

#define OT_GPTIM_ERRDIV         32768 //this needs to be hard-coded

#define OT_KTIM_IRQ_SRCLINE     BOARD_GPTIM1_PINNUM
#define OT_MACTIM_IRQ_SRCLINE   BOARD_GPTIM2_PINNUM



#define OT_TRIG1_PORT       BOARD_LEDG_PORT
#define OT_TRIG1_PINNUM     BOARD_LEDG_PINNUM
#define OT_TRIG1_PIN        BOARD_LEDG_PIN
#define OT_TRIG1_POLARITY   BOARD_LEDG_POLARITY
#define OT_TRIG2_PORT       BOARD_LEDO_PORT
#define OT_TRIG2_PINNUM     BOARD_LEDO_PINNUM
#define OT_TRIG2_PIN        BOARD_LEDO_PIN
#define OT_TRIG2_POLARITY   BOARD_LEDO_POLARITY

#define OT_TRIG(NUM, SET)   OT_TRIG##NUM##_##SET##()

#if (OT_TRIG1_POLARITY != 0)
#   define OT_TRIG1_ON()    OT_TRIG1_PORT->BSRR  = OT_TRIG1_PIN;
#   define OT_TRIG1_OFF()   OT_TRIG1_PORT->BSRR  = (OT_TRIG1_PIN << 16);
#   define OT_TRIG1_TOG()   OT_TRIG1_PORT->ODR  ^= OT_TRIG1_PIN;
#else 
#   define OT_TRIG1_ON()    OT_TRIG1_PORT->BSRR  = (OT_TRIG1_PIN <<16);
#   define OT_TRIG1_OFF()   OT_TRIG1_PORT->BSRR  = OT_TRIG1_PIN;
#   define OT_TRIG1_TOG()   OT_TRIG1_PORT->ODR  ^= OT_TRIG1_PIN;
#endif

#if (OT_TRIG2_POLARITY != 0)
#   define OT_TRIG2_ON()    OT_TRIG2_PORT->BSRR  = OT_TRIG2_PIN;
#   define OT_TRIG2_OFF()   OT_TRIG2_PORT->BSRR  = (OT_TRIG2_PIN << 16);
#   define OT_TRIG2_TOG()   OT_TRIG2_PORT->ODR  ^= OT_TRIG2_PIN;
#else 
#   define OT_TRIG2_ON()    OT_TRIG2_PORT->BSRR  = (OT_TRIG2_PIN << 16);
#   define OT_TRIG2_OFF()   OT_TRIG2_PORT->BSRR  = OT_TRIG2_PIN;
#   define OT_TRIG2_TOG()   OT_TRIG2_PORT->ODR  ^= OT_TRIG2_PIN;
#endif

static inline void BOARD_led1_on(void)      { OT_TRIG1_ON(); }
static inline void BOARD_led1_off(void)     { OT_TRIG1_OFF(); }
static inline void BOARD_led1_toggle(void)  { OT_TRIG1_TOGGLE(); }
static inline void BOARD_led2_on(void)      { OT_TRIG2_ON(); }
static inline void BOARD_led2_off(void)     { OT_TRIG2_OFF(); }
static inline void BOARD_led2_toggle(void)  { OT_TRIG2_TOGGLE(); }




#define OT_SWITCH1_PORTNUM  BOARD_SW1_PORTNUM
#define OT_SWITCH1_PORT     BOARD_SW1_PORT
#define OT_SWITCH1_PINNUM   BOARD_SW1_PINNUM
#define OT_SWITCH1_PIN      BOARD_SW1_PIN
#define OT_SWITCH1_POLARITY BOARD_SW1_POLARITY

#define OT_SWITCH2_PORTNUM  -1
#define OT_SWITCH2_PORT     -1
#define OT_SWITCH2_PINNUM   -1
#define OT_SWITCH2_PIN      -1
#define OT_SWITCH2_POLARITY -1




/** Charon STM32L0 MPipe Setup <BR>
  * ========================================================================<BR>
  * USB MPipe requires a CDC firmware library subsystem, which typically uses
  * memcpy to move data across HW buffers and such, and memcpy typically is
  * implemented using a DMA.  
  *
  * UART MPipe standard drivers REQUIRE DMA CHANNELS FOR RX AND TX.  You could
  * re-implement them without DMA, but this will impact the performance in a
  * very negative way and is not recommended.
  */
#if (MCU_CONFIG_USB == ENABLED)
// USB is mostly independent from OT, but the startup code does need to know 
// how to boost the crystal
#   if (BOARD_PARAM_HFHz != 2000000) && (BOARD_PARAM_HFHz != 3000000) \
      && (BOARD_PARAM_HFHz != 4000000) && (BOARD_PARAM_HFHz != 6000000) \
      && (BOARD_PARAM_HFHz != 8000000) && (BOARD_PARAM_HFHz != 12000000) \
      && (BOARD_PARAM_HFHz != 16000000) && (BOARD_PARAM_HFHz != 24000000)
#       error "USB requires 2, 3, 4, 6, 8, 12, 16, or 24 MHz HSE XTAL, or alternatively 16 MHz HSI+CRS"
#   endif
#   if ((BOARD_PARAM_HFppm > 50) && (BOARD_FEATURE_HFCRS != ENABLED))
#       error "USB requires an HS-XTAL with tolerance < +/- 50ppm, or alternatively usage of the CRS system"
#   endif
#   define MPIPE_USB_ID         0
#   define MPIPE_USB            USB0
#   define MPIPE_USBDP_PORT     BOARD_USB_PORT
#   define MPIPE_USBDM_PORT     BOARD_USB_PORT
#   define MPIPE_USBDP_PIN      BOARD_USB_DPPIN
#   define MPIPE_USBDM_PIN      BOARD_USB_DMPIN
#endif

#if (MCU_CONFIG_MPIPEUART == ENABLED)
#   define MPIPE_DMANUM         1
#   define MPIPE_DMA            DMA1
#   define MPIPE_UART_ID        BOARD_UART_ID
#   define MPIPE_UART_PORTNUM   BOARD_UART_PORTNUM
#   define MPIPE_UART_PORT      BOARD_UART_PORT
#   define MPIPE_UART_RXPIN     BOARD_UART_RXPIN
#   define MPIPE_UART_TXPIN     BOARD_UART_TXPIN
#   define MPIPE_RTS_PORT       BOARD_UART_PORT
#   define MPIPE_CTS_PORT       BOARD_UART_PORT
#   define MPIPE_RTS_PIN        BOARD_UART_RTSPIN
#   define MPIPE_CTS_PIN        BOARD_UART_CTSPIN
#   define MPIPE_UART_PINS      (MPIPE_UART_RXPIN | MPIPE_UART_TXPIN)
#   if (MPIPE_UART_ID != 1)
#       error "MPIPE UART must be on USART1 for this board."
#   endif
#   define MPIPE_UART       USART1
#   define MPIPE_DMA_RXCHAN_ID  5
#   define MPIPE_DMA_TXCHAN_ID  4
#   define __USE_DMA1_CHAN5
#   define __USE_DMA1_CHAN4
#endif

#if (MCU_CONFIG_MPIPEI2C == ENABLED)
#   define MPIPE_I2C_ID         BOARD_I2C_ID
#   define MPIPE_I2C            I2C2
#   define MPIPE_I2C_PORTNUM    BOARD_I2C_PORTNUM
#   define MPIPE_I2C_PORT       BOARD_I2C_PORT
#   define MPIPE_I2C_RXPIN      BOARD_I2C_SCLPIN
#   define MPIPE_I2C_TXPIN      BOARD_I2C_SDAPIN
#   define MPIPE_I2C_PINS       (MPIPE_I2C_RXPIN | MPIPE_I2C_TXPIN)
#   if (MPIPE_I2C_ID != 2)
#       error "MPIPE I2C must be on I2C2 for this board."
#   endif
#   ifndef MPIPE_DMANUM
#       define MPIPE_DMANUM         1
#       define MPIPE_DMA            DMA1
#       define MPIPE_DMA_RXCHAN_ID  5
#       define MPIPE_DMA_TXCHAN_ID  4
#       define __USE_DMA1_CHAN5
#       define __USE_DMA1_CHAN4
#   endif
#endif












/** Boilerplate STM32L DMA Memcpy Setup <BR>
  * ========================================================================<BR>
  * Memcpy should have a low DMA priority, such as Channel7.  It is a MEM2MEM
  * function of the DMA, so it can take any channel.  If there is another user
  * of DMA Channel 7, then the potential throughput of that communication
  * interface is limited by the duty time of memcpy.
  */
#if (MCU_CONFIG_MEMCPYDMA != ENABLED)
#   warning "Not using DMA for MEMCPY, a sub-optimal design choice."
#elif !defined(__USE_DMA1_CHAN7)
#   define MEMCPY_DMA_CHAN_ID 7
#   define MEMCPY_DMA       DMA1
#   define MEMCPY_DMACHAN   DMA1_Channel7
#   define __USE_DMA1_CHAN7
#elif !defined(__USE_DMA1_CHAN6)
#   define MEMCPY_DMA_CHAN_ID    6
#   define MEMCPY_DMA       DMA1
#   define MEMCPY_DMACHAN   DMA1_Channel6
#   define __USE_DMA1_CHAN6
#elif !defined(__USE_DMA1_CHAN5)
#   define MEMCPY_DMA_CHAN_ID    5
#   define MEMCPY_DMA       DMA1
#   define MEMCPY_DMACHAN   DMA1_Channel5
#   define __USE_DMA1_CHAN5
#elif !defined(__USE_DMA1_CHAN4)
#   define MEMCPY_DMA_CHAN_ID    4
#   define MEMCPY_DMA       DMA1
#   define MEMCPY_DMACHAN   DMA1_Channel4
#   define __USE_DMA1_CHAN4
#elif !defined(__USE_DMA1_CHAN3)
#   define MEMCPY_DMA_CHAN_ID    3
#   define MEMCPY_DMA       DMA1
#   define MEMCPY_DMACHAN   DMA1_Channel3
#   define __USE_DMA1_CHAN3
#elif !defined(__USE_DMA1_CHAN2)
#   define MEMCPY_DMA_CHAN_ID    2
#   define MEMCPY_DMA       DMA1
#   define MEMCPY_DMACHAN   DMA1_Channel2
#   define __USE_DMA1_CHAN2
#elif !defined(__USE_DMA1_CHAN1)
#   define MEMCPY_DMA_CHAN_ID    1
#   define MEMCPY_DMA       DMA1
#   define MEMCPY_DMACHAN   DMA1_Channel1
#   define __USE_DMA1_CHAN1
#else
#   error "There is no available DMA Channel to allocate to MEMCPY."
#endif






/** EXTI Line Utilization declaration <BR>
  * ========================================================================<BR>
  * On this module, all EXTIs are available to the user app or system.
  */
#ifndef __USE_EXTI0
#   define __USE_EXTI0
#endif
#ifndef __USE_EXTI1
#   define __USE_EXTI1
#endif
#ifndef __USE_EXTI2
#   define __USE_EXTI2
#endif
#ifndef __USE_EXTI3
#   define __USE_EXTI3
#endif
#ifndef __USE_EXTI4
#   define __USE_EXTI4
#endif
#ifndef __USE_EXTI5
#   define __USE_EXTI5
#endif
#ifndef __USE_EXTI6
#   define __USE_EXTI6
#endif
#ifndef __USE_EXTI7
#   define __USE_EXTI7
#endif
#ifndef __USE_EXTI8
#   define __USE_EXTI8
#endif
#ifndef __USE_EXTI9
#   define __USE_EXTI9
#endif
#ifndef __USE_EXTI10
#   define __USE_EXTI10
#endif
#ifndef __USE_EXTI11
#   define __USE_EXTI11
#endif
#ifndef __USE_EXTI12
#   define __USE_EXTI12
#endif
#ifndef __USE_EXTI13
#   define __USE_EXTI13
#endif
#ifndef __USE_EXTI14
#   define __USE_EXTI14
#endif
#ifndef __USE_EXTI15
#   define __USE_EXTI15
#endif


//Wipe-out some temporary constants which have generic names
#undef _CRYPCLK_N
#undef _CRYPCLK_LP 
#undef _RNGCLK_N 
#undef _RNGCLK_LP 
#undef _TOUCHCLK_N 
#undef _TOUCHCLK_LP 
#undef _CRCCLK_N
#undef _CRCCLK_LP 
#undef _SRAMCLK_LP 
#undef _MIFCLK_N
#undef _MIFCLK_LP 
#undef _DMACLK_N 
#undef _DMACLK_LP 
#undef _DMACLK_DYNAMIC
#undef _GPIOCLK_N 
#undef _GPIOCLK_SU 
#undef _GPIOCLK_LP 
#undef _IOPENR_STARTUP 
#undef _IOPENR_RUNTIME 





#endif
