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
  * @file       /board/cc13xx/launchpad_CC1310.h
  * @author     JP Norair
  * @version    R100
  * @date       29 Oct 2016
  * @brief      Board Configuration for TI CC1310 Launchpad
  * @ingroup    Platform
  *
  * Do not include this file, include /platform/config.h
  *
  * SPECIAL NOTE: search for "note BOARD Macro" lines in this file.  These
  * must be ported to other STM32L boards, because the OpenTag STM32L platform 
  * implementation depends on them.
  *
  ******************************************************************************
  */
  

#ifndef __launchpad_CC1310_H
#define __launchpad_CC1310_H

/// MCU definition for the board must be the first thing
#define __CC1310F128RGZ__

/// Controls whether you actually use the radio.
/// It is possible to test some systems just using a radio simulation in SRAM.
#define __USE_RADIO

#include <app/app_config.h>
#include <platform/hw/CC13xx_config.h>
#include <platform/interrupts.h>


#ifdef __USE_RADIO
#   ifdef __NULL_RADIO__
#       include <io/null_radio/config.h>
#   else
#       include <io/cc1310/config.h>
#   endif
#endif



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




/** Additional RF Front End Parameters and Settings <BR>
  * ========================================================================<BR>
  * We assume you are using the included antenna and matching circuit, which 
  * has a 50% efficiency (-3dB) with 1dB insertion loss
  */
#ifdef __USE_RADIO
#   define RF_PARAM_BAND   866              //Can be 866 or 915
#   define RF_HDB_ATTEN    8                //Half dB attenuation (units = 0.5dB), used to scale TX power
#   define RF_HDB_RXATTEN  6
#   define RF_RSSI_OFFSET  RF_HDB_RXATTEN   //Offset applied to RSSI calculation
#endif



/** MCU Configuration settings      <BR>
  * ========================================================================<BR>
  * Implemented capability variants on this board/build
  */
#define MCU_CONFIG(VAL)                 MCU_CONFIG_##VAL   // FEATURE 
#define MCU_CONFIG_MULTISPEED           DISABLED                            // Automatic Clock Speed throttling
#define MCU_CONFIG_MAPEEPROM            NOT_AVAILABLE                       // CC13xx does not have built-in EEPROM
#define MCU_CONFIG_MPIPECDC             NOT_AVAILABLE                       // CC13xx does not have built-in USB
#define MCU_CONFIG_MPIPEUART            (MCU_CONFIG_MPIPECDC != ENABLED)    // UART MPipe Implementation
#define MCU_CONFIG_MPIPEI2C             DISABLED                            // I2C MPipe Implementation
#define MCU_CONFIG_MEMCPYDMA            ENABLED                             // MEMCPY DMA must be lowest priority of DMAs
#define MCU_CONFIG_USB                  (MCU_CONFIG_MPIPECDC == ENABLED)
#define MCU_CONFIG_VOLTLEVEL            0  // 2=Boost, 1=Normal, 0=Auto (Auto mode uses Normal voltage unless high power is selected in SW
#define MCU_CONFIG_CACHE                1  // 0=OFF, 1=ON, 2=Split




/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.  
  *
  * On the CC13xx systems, the FS is stored in FLASH but typically all of the
  * files are mirrored in SRAM.  This is much more efficient than maintaining
  * files directly in Flash, especially on the CC1310 which has large flash
  * pages (4KB each).
  *
  * As Flash pages are 4KB, the size of the filesystem should be 4KB or 8KB.
  * 12KB is also possible, although this reduces RAM available to the system
  * to 8KB.  Often this is enough, but for advanced applications it may not be.
  * 
  * Default filesystem size is 8KB, which is more than enough.  If you set the
  * FS to 4 or 8 KB, it will be stored in the 4KB RAM sectors.  If you set it 
  * to 12KB, it will be stored in the 6KB RAM sectors.  These sectors will be
  * retained in low power modes.
  *
  * @note This configuration does not support A/B Firmware update features. 
  * Haystack's premium libraries for OpenTag support A/B Firmware updating.
  */

// You can comment-out these if you want to use chip defaults
#ifndef SRAM_SIZE
#   define SRAM_SIZE            (20*1024)
#endif
#ifndef EEPROM_SIZE
#   define EEPROM_SIZE          (0*1024)
#endif
#ifndef FLASH_SIZE
#   define FLASH_SIZE           (128*1024)
#endif
#ifndef EEPROM_SAVE_SIZE
#   define EEPROM_SAVE_SIZE     (0)
#endif

// Using FLASH: FS is stored at the back of the flash.
#define FLASH_NUM_PAGES         (FLASH_SIZE / FLASH_PAGE_SIZE)      // FLASH_PAGE_SIZE comes platform/hw/...config.h
#define FLASH_FS_PAGES          2
#define FLASH_FS_FALLOWS        0
#define FLASH_FS_ALLOC          (FLASH_PAGE_SIZE * FLASH_FS_PAGES) 
#define FLASH_FS_ADDR           (FLASH_START_ADDR + FLASH_SIZE - FLASH_FS_ALLOC)









/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  * 1. Buttons 1 and 2 are supported on DIO13 and DIO14 respectively
  * 
  * 2. Two LEDs are defined on DIO6 and DIO7.
  *    <LI> TRIG1 (Red LED) is on DIO6.  It typically indicates 
  *         that the radio is on and in RX mode.</LI>
  *    <LI> TRIG2 (Green LED) is on DIO7.  It typically indicates
  *         that the radio is on and in TX mode.</LI>
  *
  * 3. MPIPE can support UART TX/RX (no flow control) or I2C.  MPIPE supports
  *    a "break" mode, in both I2C and UART interfaces, where a break character 
  *    is transmitted by the client ahead of the packet in order to wakeup the 
  *    system.  This reduces power requirements.
  * 
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_PARAM(VAL)                BOARD_PARAM_##VAL

#define BOARD_FEATURE_MPIPE             ENABLED
#define BOARD_FEATURE_USBCONVERTER      BOARD_FEATURE_MPIPE         // Is UART connected via USB converter?
#define BOARD_FEATURE_MPIPE_BREAK       ENABLED                     // Send/receive leading break for wakeup

#define BOARD_FEATURE_LFXTAL            ENABLED                                 // LF XTAL used as Clock source
#define BOARD_FEATURE_HFXTAL            DISABLED                                // HF XTAL used as Clock source
#define BOARD_FEATURE_HFBYPASS          DISABLED                                // Use an externally driven oscillator
//#define BOARD_FEATURE_RFXTAL            DISABLED                                // XTAL for RF chipset
//#define BOARD_FEATURE_RFXTALOUT         DISABLED
#define BOARD_FEATURE_FULLSPEED         ENABLED                                 // 24 MHz
#define BOARD_FEATURE_FULLXTAL          DISABLED
#define BOARD_FEATURE_FLANKSPEED        (MCU_CONFIG_MULTISPEED == ENABLED)      // 48 MHz
#define BOARD_FEATURE_FLANKXTAL         DISABLED

#define BOARD_PARAM_TRIGS               2
#define BOARD_PARAM_LFHz                32768
#define BOARD_PARAM_LFtol               0.00002
#define BOARD_PARAM_HFHz                48000000
#define BOARD_PARAM_HFtol               0.02
#define BOARD_PARAM_HFppm               20000

// These are defined at FULL SPEED
#define BOARD_PARAM_AHBCLKDIV           1                       // AHB Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB2CLKDIV          1                       // APB2 Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB1CLKDIV          1                       // APB1 Clk = Main CLK / AHBCLKDIV

// JTAG and SWO Interfaces:
// TI puts TMS and TCKC on reserved pins, but TDO and TDI are muxed on DIO16 and DIO17
// Likewise, SWO is on DIO18.  Set this if you plan to use SWO.
#define BOARD_JTDO_PINNUM               16
#define BOARD_JTDO_PIN                  (1<<BOARD_JTDO_PINNUM)
#define BOARD_JTDI_PINNUM               17
#define BOARD_JTDI_PIN                  (1<<BOARD_JTDI_PINNUM)
#define BOARD_TRACESWO_PINNUM           18
#define BOARD_TRACESWO_PIN              0   //(1<<BOARD_TRACESWO_PINNUM)
#define BOARD_JTAG_PINS                 (BOARD_JTDO_PIN | BOARD_JTDI_PIN | BOARD_TRACESWO_PIN)

// SW1, SW2 are implemented on DIO12, DIO13 as active low.
#define BOARD_SW1_PINNUM                12
#define BOARD_SW1_PIN                   (1<<BOARD_SW1_PINNUM)
#define BOARD_SW1_POLARITY              0
#define BOARD_SW1_PULLING               0
#define BOARD_SW2_PINNUM                13
#define BOARD_SW2_PIN                   (1<<BOARD_SW2_PINNUM)
#define BOARD_SW2_POLARITY              0
#define BOARD_SW2_PULLING               0
#define BOARD_SW_PINS                   (BOARD_SW1_PIN | BOARD_SW2_PIN)

// LED interface is implemented on DIO7 and DIO8, which means you
// need to connect your own LEDS to these pins.
// "LEDO" is for "LED Orange" although TI uses a Red LED
#define BOARD_LEDG_PINNUM               7
#define BOARD_LEDG_PIN                  (1<<BOARD_LEDG_PINNUM)
#define BOARD_LEDG_POLARITY             1
#define BOARD_LEDO_PINNUM               8
#define BOARD_LEDO_PIN                  (1<<BOARD_LEDO_PINNUM)
#define BOARD_LEDO_POLARITY             1
#define BOARD_LED_PINS                  (BOARD_LEDG_PIN | BOARD_LEDO_PIN)

// TEST0, TEST1 pins are on DIO21, DIO22.
// They are not necessary, but Haystack uses them for manufacturing tests.
// For application purposes, these pins are available for any sort of usage.
#define BOARD_TEST0_PINNUM              21
#define BOARD_TEST0_PIN                 (1<<BOARD_TEST0_PINNUM)
#define BOARD_TEST0_POLARITY            0
#define BOARD_TEST0_PULLING             1
#define BOARD_TEST1_PINNUM              22
#define BOARD_TEST1_PIN                 (1<<BOARD_TEST1_PINNUM)
#define BOARD_TEST1_POLARITY            0
#define BOARD_TEST1_PULLING             1

// Main serial connections, used at least by MPIPE

// I2C on DIO4 & 5
#define BOARD_I2C_ID                    0
#define BOARD_I2C_SCLPINNUM             4
#define BOARD_I2C_SDAPINNUM             5
#define BOARD_I2C_SCLPIN                (1<<BOARD_I2C_SCLPINNUM)
#define BOARD_I2C_SDAPIN                (1<<BOARD_I2C_SDAPINNUM)
#define BOARD_I2C_PINS                  (BOARD_I2C_SCLPIN | BOARD_I2C_SDAPIN)

// UART on DIO2 & 3
#define BOARD_UART_ID                   0
#define BOARD_UART_TXPINNUM             3
#define BOARD_UART_RXPINNUM             2
#define BOARD_UART_TXPIN                (1<<BOARD_UART_TXPINNUM)
#define BOARD_UART_RXPIN                (1<<BOARD_UART_RXPINNUM)
#define BOARD_UART_PINS                 (BOARD_UART_TXPIN | BOARD_UART_RXPIN)

// ADC Analog inputs:
#define BOARD_ADC_PINS                  8
#define BOARD_ADC_0CHAN                 
#define BOARD_ADC_0PINNUM               23
#define BOARD_ADC_0PIN                  (1<<BOARD_ADC_0PINNUM)
#define BOARD_ADC_1CHAN                 
#define BOARD_ADC_1PINNUM               24
#define BOARD_ADC_1PIN                  (1<<BOARD_ADC_1PINNUM)
#define BOARD_ADC_2CHAN                 
#define BOARD_ADC_2PINNUM               25
#define BOARD_ADC_2PIN                  (1<<BOARD_ADC_2PINNUM)
#define BOARD_ADC_3CHAN                 
#define BOARD_ADC_3PINNUM               26
#define BOARD_ADC_3PIN                  (1<<BOARD_ADC_3PINNUM)
#define BOARD_ADC_4CHAN                 
#define BOARD_ADC_4PINNUM               27
#define BOARD_ADC_4PIN                  (1<<BOARD_ADC_4PINNUM)
#define BOARD_ADC_5CHAN                 
#define BOARD_ADC_5PINNUM               28
#define BOARD_ADC_5PIN                  (1<<BOARD_ADC_5PINNUM)
#define BOARD_ADC_6CHAN                 
#define BOARD_ADC_6PINNUM               29
#define BOARD_ADC_6PIN                  (1<<BOARD_ADC_6PINNUM)
#define BOARD_ADC_7CHAN                 
#define BOARD_ADC_7PINNUM               30
#define BOARD_ADC_7PIN                  (1<<BOARD_ADC_6PINNUM)
#define BOARD_ADC_PINS                  (BOARD_ADC_0PIN | BOARD_ADC_1PIN | BOARD_ADC_2PIN | BOARD_ADC_3PIN \
                                         BOARD_ADC_4PIN | BOARD_ADC_5PIN | BOARD_ADC_6PIN | BOARD_ADC_7PIN)


// Timer IO: 
// The Launchpad form factor only specifies a single PWM on DIO12.  Since OpenTag doesn't 
// use any PWM, it's not specified, and the application must deal with it.
// 
// OpenTag does, however, utilize Timer3 for certain internal purposes.  In particular,
// it uses TIM3B.  You can use TIM3A for your own purposes as long as you don't change
// the clocking or other global TIM3 control settings.
#define BOARD_TIMA_PORTNUM              1
#define BOARD_TIMA_PORT                 GPIOB
#define BOARD_TIMA_ID                   3
#define BOARD_TIMA                      TIM3
#define BOARD_TIMA_CHANS                2
#define BOARD_TIMA_1CHAN                1
#define BOARD_TIMA_2CHAN                2
#define BOARD_TIMA_1PINNUM              4
#define BOARD_TIMA_2PINNUM              5
#define BOARD_TIMA_1PIN                 (1<<BOARD_TIMA_1PINNUM)
#define BOARD_TIMA_2PIN                 (1<<BOARD_TIMA_2PINNUM)




/** BOARD Macros (most of them, anyway) <BR>
  * ========================================================================<BR>
  * BOARD Macros are required by the OpenTag CC13XX platform implementation.
  * We can assume certain things at the platform level, but not everything,
  * which is why BOARD Macros are required.
  *
  * If your board is implementing a radio (extremely likely), there are likely
  * some additional Board macros down near the radio configuration section.
  */

/// Clocking parameters: 
/// CC13xx has a reasonably straightforward peripheral clocking model.
/// We always clock the peripherals at 24MHz, which is either:
/// - XTAL * 1, if crystal is used as system clock
/// - HSOSC / 2, if crystal is not used.



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
#define _GPIOCLK_N       (RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN | RCC_IOPENR_GPIODEN)
#define _GPIOCLK_SU      (_GPIOCLK_N | RCC_IOPENR_GPIOHEN)
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
///      - Radio EXTIs: {GPIO-0, -1, -2, -3} : {PA10, PB3, PB5, PB4}
static inline void BOARD_EXTI_STARTUP(void) {
    
    // EXTI0-3: A0, A1, B2, RFGPIO-1 (B3)
    SYSCFG->EXTICR[0]   = (0 << 0) \
                        | (0 << 4) \
                        | (1 << 8) \
                        | (BOARD_RFGPIO_1PORTNUM << 12);
    
    // EXTI4-7: RFGPIO-3 (B4), RFGPIO-2 (B5), B6, C7
    SYSCFG->EXTICR[1]   = (BOARD_RFGPIO_3PORTNUM << 0) \
                        | (BOARD_RFGPIO_2PORTNUM << 4) \
                        | (1 << 5) \
                        | (2 << 12);
                        
    // EXTI8-11: A8, A9, RFGPIO-0 (A10), A11
    SYSCFG->EXTICR[2]   = (0 << 0) \
                        | (0 << 4) \
                        | (BOARD_RFGPIO_0PORTNUM << 8) \
                        | (0 << 12);

    // EXTI12-15: A12, SW1 (C13), B14, A15
    SYSCFG->EXTICR[3]   = (0 << 0) \
                        | (2 << 4) \
                        | (1 << 8) \
                        | (0 << 12);
}



///@note BOARD Macro for initializing GPIO ports at startup, pursuant to the
///      connections in the schematic of this board.  This funciotn
static inline void BOARD_PORT_STARTUP(void) {  
    /// Initialize ports/pins exclusively used within this platform module.
    /// A. Trigger Pins
    /// B. Random Number ADC pins: A Zener can be used to generate noise.
    
    /// Configure Port A IO.  
    /// Port A is used for internal features and .
    // - A0,1,4 are used for Analog Inputs
    // - A2 is UART-TX, which is ALT push-pull output
    // - A3 is UART-RX, which is ALT push-pull input
    // - A5:7 are SPI bus, set to ALT.
    // - A8:10 are inputs (often used with radio)
    // - A11:12 are inputs which are not available on the Arduino pinout (USB)
    // - A13:14 are SWD, set to ALT
#   ifdef __USE_RADIO
    GPIOA->BSRR     = BOARD_RFSPI_CSNPIN;
#   endif

    GPIOA->MODER    = (GPIO_MODER_ANALOG << (0*2)) \
                    | (GPIO_MODER_ANALOG << (1*2)) \
                    | (GPIO_MODER_ALT << (2*2)) \
                    | (GPIO_MODER_ALT << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_ALT << (5*2)) \
                    | (GPIO_MODER_ALT << (6*2)) \
                    | (GPIO_MODER_ALT << (7*2)) \
                    | (GPIO_MODER_IN  << (8*2)) \
                    | (GPIO_MODER_IN  << (9*2)) \
                    | (GPIO_MODER_IN  << (10*2)) \
                    | (GPIO_MODER_IN  << (11*2)) \
                    | (GPIO_MODER_IN  << (12*2)) \
                    | (GPIO_MODER_ALT << (13*2)) \
                    | (GPIO_MODER_ALT << (14*2)) \
                    | (GPIO_MODER_IN  << (15*2));

    GPIOA->OTYPER   = 0;
    
    GPIOA->OSPEEDR  = (GPIO_OSPEEDR_2MHz << (2*2)) \
                    | (GPIO_OSPEEDR_2MHz << (3*2)) \
                    | (GPIO_OSPEEDR_10MHz << (6*2)) \
                    | (GPIO_OSPEEDR_10MHz << (7*2)) \
                    | (GPIO_OSPEEDR_40MHz << (13*2)) \
                    | (GPIO_OSPEEDR_40MHz << (14*2));
    
    GPIOA->PUPDR    = (1 << (3*2)) \
                    | (1 << (13*2)) | (2 << (14*2));
    

#   ifndef __USE_RADIO
    GPIOA->AFR[0]   = (7 << (BOARD_UART_TXPINNUM*4)) \
                    | (7 << (BOARD_UART_RXPINNUM*4));
#   else
    GPIOA->AFR[0]   = (7 << (BOARD_UART_TXPINNUM*4)) \
                    | (7 << (BOARD_UART_RXPINNUM*4)) \
                    | (5 << ((BOARD_RFSPI_MOSIPINNUM)*4)) \
                    | (5 << ((BOARD_RFSPI_MISOPINNUM)*4)) \
                    | (5 << ((BOARD_RFSPI_SCLKPINNUM)*4));
#   endif

    
    GPIOA->AFR[1]   = (10 << ((BOARD_USB_DMPINNUM-8)*4)) \
                    | (10 << ((BOARD_USB_DPPINNUM-8)*4));

    /* */

    /// Configure Port B IO.
    /// Port B is used for external (module) IO.
    // - B0 is an Analog Input
    // - B1:2 are unused and set to Analog mode
    // - B3 is by default an input
    // - B4 is a PWM output, set to Analog until configured
    // - B5 is used as a radio input by default
    // - B6 is used as SPI-CS, an output
    // - B7 is unused and set to Analog mode
    // - B8:9 are setup for I2C bus: They start as output with Open Drain
    // - B10 is a PWM output, set to Analog until configured
    // - B11:15 are unused and set to Analog mode
    GPIOB->OTYPER   = (1 << (8)) | (1 << (9));
    GPIOB->BSRR     = (1 << (8)) | (1 << (9));

    GPIOB->MODER    = (GPIO_MODER_ANALOG << (0*2)) \
                    | (GPIO_MODER_ANALOG << (1*2)) \
                    | (GPIO_MODER_ANALOG << (2*2)) \
                    | (GPIO_MODER_IN     << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_IN     << (5*2)) \
                    | (GPIO_MODER_OUT    << (6*2)) \
                    | (GPIO_MODER_ANALOG << (7*2)) \
                    | (GPIO_MODER_OUT    << (8*2)) \
                    | (GPIO_MODER_OUT    << (9*2)) \
                    | (GPIO_MODER_ANALOG << (10*2)) \
                    | (GPIO_MODER_ANALOG << (11*2)) \
                    | (GPIO_MODER_ANALOG << (12*2)) \
                    | (GPIO_MODER_ANALOG << (13*2)) \
                    | (GPIO_MODER_ANALOG << (14*2)) \
                    | (GPIO_MODER_ANALOG << (15*2));
    
    GPIOB->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (8*2)) \
                    | (GPIO_OSPEEDR_10MHz << (9*2));
    
    /// Configure Port C IO.
    /// Port C is used only for USB sense and 32kHz crystal driving
    // - C0:1 are analog inputs
    // - C2:6 are unused and set to analog
    // - C7 is the radio shutdown push-pull output
    // - C8:12 are unused and set to analog
    // - C13 is the user button, set to input HiZ
    // - C14:15 are 32kHz crystal driving, set to ALT
#   ifdef __USE_RADIO
    GPIOC->BSRR     = BOARD_RFCTL_SDNPIN;
#   endif

    GPIOC->MODER    = (GPIO_MODER_ANALOG << (0*2)) \
                    | (GPIO_MODER_ANALOG << (1*2)) \
                    | (GPIO_MODER_ANALOG << (2*2)) \
                    | (GPIO_MODER_ANALOG << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_ANALOG << (5*2)) \
                    | (GPIO_MODER_ANALOG << (6*2)) \
                    | (GPIO_MODER_OUT    << (7*2)) \
                    | (GPIO_MODER_ANALOG << (8*2)) \
                    | (GPIO_MODER_ANALOG << (9*2)) \
                    | (GPIO_MODER_ANALOG << (10*2)) \
                    | (GPIO_MODER_ANALOG << (11*2)) \
                    | (GPIO_MODER_ANALOG << (12*2)) \
                    | (GPIO_MODER_IN     << (13*2)) \
                    | (GPIO_MODER_OUT    << (14*2)) \
                    | (GPIO_MODER_ALT    << (15*2));
    
    /// Configure Port D I/O
    /// Port D is completely unused and set by default to Analog
    GPIOD->MODER    = 0xFFFFFFFF;
    
    // Assert Port H as HiZ
    GPIOH->MODER    = (GPIO_MODER_IN << (0*2))
                    | (GPIO_MODER_IN << (1*2));
    
    /// Configure Port H for Crystal Bypass
    /// By default it is set to HiZ input.  It is changed on-demand in FW
    
    // Disable GPIOC and GPIOH, they are never altered after this.
    RCC->IOPENR = _IOPENR_RUNTIME;
}





static inline void BOARD_RFSPI_CLKON(void) {
#ifdef __USE_RADIO
    BOARD_RFSPI_PORT->MODER &= ~((3 << (BOARD_RFSPI_SCLKPINNUM*2)) \
                            | (3 << (BOARD_RFSPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_RFSPI_MOSIPINNUM*2)) );
    
    BOARD_RFSPI_PORT->MODER |= (GPIO_MODER_ALT << (BOARD_RFSPI_SCLKPINNUM*2)) \
                            | (GPIO_MODER_ALT << (BOARD_RFSPI_MISOPINNUM*2)) \
                            | (GPIO_MODER_ALT << (BOARD_RFSPI_MOSIPINNUM*2));
    
  //BOARD_RFSPI_PORT->MODER ^= (3 << (BOARD_RFSPI_SCLKPINNUM*2)) \
                            | (2 << (BOARD_RFSPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_RFSPI_MOSIPINNUM*2));
#endif
}

static inline void BOARD_RFSPI_CLKOFF(void) {
#ifdef __USE_RADIO
    BOARD_RFSPI_PORT->MODER &= ~((3 << (BOARD_RFSPI_SCLKPINNUM*2)) \
                            | (3 << (BOARD_RFSPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_RFSPI_MOSIPINNUM*2)) );
    
    BOARD_RFSPI_PORT->MODER |= (GPIO_MODER_OUT << (BOARD_RFSPI_SCLKPINNUM*2)) \
                            | (GPIO_MODER_IN << (BOARD_RFSPI_MISOPINNUM*2)) \
                            | (GPIO_MODER_OUT << (BOARD_RFSPI_MOSIPINNUM*2));
    
  //BOARD_RFSPI_PORT->MODER ^= (3 << (BOARD_RFSPI_SCLKPINNUM*2)) \
                            | (2 << (BOARD_RFSPI_MISOPINNUM*2)) \
                            | (3 << (BOARD_RFSPI_MOSIPINNUM*2));
#endif
}



#include <platform/timers.h>    // for systim_stop_clocker()
static inline void BOARD_STOP(ot_int code) {
/// code comes from sys_sig_powerdown, but it is usually 0-3.
/// For all STM32L devices, 3 is full-idle and 2 is radio-active-idle.  
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


   

///@note BOARD Macros for Radio module interrupt vectoring.  Connect these to
///      the radio interface driver you are using.  Check the schematic of your
///      board to see where the Radio IRQ lines are routed.  Also check the 
///      radio interface header documentation (it's really quite simple). 
///      These Macros will get called in the universal EXTI interrupt handler,
///      typically implemented in platform_isr_STM32L.c
//#define BOARD_RADIO_EXTI0_ISR(); 
//#define BOARD_RADIO_EXTI1_ISR();   
//#define BOARD_RADIO_EXTI2_ISR();
//#define BOARD_RADIO_EXTI3_ISR();
//#define BOARD_RADIO_EXTI4_ISR();
//#define BOARD_RADIO_EXTI5_ISR();
//#define BOARD_RADIO_EXTI6_ISR();     
//#define BOARD_RADIO_EXTI7_ISR();     
//#define BOARD_RADIO_EXTI8_ISR()     spirit1_irq0_isr()
//#define BOARD_RADIO_EXTI9_ISR()     spirit1_irq1_isr()
//#define BOARD_RADIO_EXTI10_ISR()    spirit1_irq2_isr()
//#define BOARD_RADIO_EXTI11_ISR();
//#define BOARD_RADIO_EXTI12_ISR();
//#define BOARD_RADIO_EXTI13_ISR();
//#define BOARD_RADIO_EXTI14_ISR();
//#define BOARD_RADIO_EXTI15_ISR();


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
#define PLATFORM_PLLCLOCK_OUT       ((BOARD_PARAM_RFHz/BOARD_PARAM_RFdiv)*BOARD_PARAM_PLLmult)
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




/** Nucleo STM32L0 MPipe Setup <BR>
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









/** Boilerplate STM32L SPIRIT1-RF Setup <BR>
  * ========================================================================<BR>
  * The SPIRIT1 interface uses an SPI (ideally SPI1 because it is attached to
  * the high-speed APB2 bus).  It also typically uses a DMA for bulk transfers,
  * during which the core can be shut-off for reducing power.
  */
 
#ifdef __USE_RADIO
#define RADIO_SPI_ID    BOARD_RFSPI_ID
#if (BOARD_RFSPI_ID != 1)
#   error "RADIO_SPI must be SPI1 on this board"
#endif
#define RADIO_SPI    SPI1
#if (defined(__USE_DMA1_CHAN2) || defined(__USE_DMA1_CHAN3))
#   error "RADIO SPI is SPI1, which needs DMA Ch2 & Ch3, but they are used."
#else
#   define __USE_DMA1_CHAN2
#   define __USE_DMA1_CHAN3
#   define __N_ISR_DMA1_Channel2
#   define __N_ISR_DMA1_Channel3
#endif

#define RADIO_SPI_PORT              BOARD_RFSPI_PORT
#define RADIO_SPIMOSI_PORT          BOARD_RFSPI_PORT
#define RADIO_SPIMISO_PORT          BOARD_RFSPI_PORT
#define RADIO_SPICLK_PORT           BOARD_RFSPI_PORT
#define RADIO_SPICS_PORT            BOARD_RFSPI_CSNPORT
#define RADIO_SPIMOSI_PIN           BOARD_RFSPI_MOSIPIN
#define RADIO_SPIMISO_PIN           BOARD_RFSPI_MISOPIN
#define RADIO_SPICLK_PIN            BOARD_RFSPI_SCLKPIN
#define RADIO_SPICS_PIN             BOARD_RFSPI_CSNPIN

#define RADIO_IRQ0_SRCPORT          BOARD_RFGPIO_0PORTNUM
#define RADIO_IRQ1_SRCPORT          BOARD_RFGPIO_1PORTNUM
#define RADIO_IRQ2_SRCPORT          BOARD_RFGPIO_2PORTNUM
#define RADIO_IRQ3_SRCPORT          BOARD_RFGPIO_3PORTNUM
#define RADIO_IRQ0_SRCLINE          BOARD_RFGPIO_0PINNUM
#define RADIO_IRQ1_SRCLINE          BOARD_RFGPIO_1PINNUM
#define RADIO_IRQ2_SRCLINE          BOARD_RFGPIO_2PINNUM
#define RADIO_IRQ3_SRCLINE          -1

#define RADIO_SDN_PORT              BOARD_RFCTL_PORT
#define RADIO_IRQ0_PORT             BOARD_RFGPIO_0PORT
#define RADIO_IRQ1_PORT             BOARD_RFGPIO_1PORT
#define RADIO_IRQ2_PORT             BOARD_RFGPIO_2PORT
#define RADIO_IRQ3_PORT             BOARD_RFGPIO_3PORT
#define RADIO_SDN_PIN               BOARD_RFCTL_SDNPIN
#define RADIO_IRQ0_PIN              BOARD_RFGPIO_0PIN
#define RADIO_IRQ1_PIN              BOARD_RFGPIO_1PIN
#define RADIO_IRQ2_PIN              BOARD_RFGPIO_2PIN
#define RADIO_IRQ3_PIN              BOARD_RFGPIO_3PIN
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
