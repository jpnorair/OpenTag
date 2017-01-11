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
  * @file       /board/stm32l0xx/nucleo_LRWAN1.h
  * @author     JP Norair
  * @version    R100
  * @date       29 Oct 2016
  * @brief      Board Configuration for Nucleo STM32L073 with LoRa module
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
  

#ifndef __nucleo_LRWAN1_H
#define __nucleo_LRWAN1_H

/// MCU definition for the board must be the first thing
#define __STM32L073RZ__

#include <app/app_config.h>
#include <platform/hw/STM32L0xx_config.h>
#include <platform/interrupts.h>

/// This is a major difference between the LRWAN1 system and the unladen Nucleo
#if OT_FEATURE(M2)
#   define __USE_RADIO
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


/// Configuration of system with or without Radio
#if defined(__USE_RADIO)
#   ifdef __NULL_RADIO__
//#       include <io/radio_null/config.h>
#   else
#		define __SX127x__
#		define __SX1272__
#       include <io/sx127x/config.h>
#   endif
#endif







/** Additional RF Front End Parameters and Settings <BR>
  * ========================================================================<BR>
  * We assume you are using the included antenna and matching circuit, which 
  * has a 50% efficiency (-3dB) with 1dB insertion loss
  */
#ifdef __USE_RADIO
//#   define RF_PARAM_BAND    866              //Can be 866 or 915
#   define RF_PARAM_BAND    915
#   define RF_HDB_ATTEN     8                //Half dB attenuation (units = 0.5dB), used to scale TX power
#   define RF_HDB_RXATTEN   6
#   define RF_RSSI_OFFSET   RF_HDB_RXATTEN   //Offset applied to RSSI calculation
#endif



/** MCU Configuration settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the STM32L variants on this board/build
  *
  * On this board you can use USB or UART for MPipe, I2C is not implemented
  * yet.  To select USB, make sure "MCU_CONFIG_MPIPECDC" is set to "ENABLED".
  * To use UART, set "MCU_CONFIG_MPIPECDC" to "DISABLED" and the script 
  * below will use UART instead.
  *
  */
#define MCU_CONFIG(VAL)                 MCU_CONFIG_##VAL   // FEATURE 
#define MCU_CONFIG_MULTISPEED           DISABLED                            // Allows usage of MF-HF clock boosting
#define MCU_CONFIG_MAPEEPROM            DISABLED
#define MCU_CONFIG_MPIPECDC             DISABLED                            // USB-CDC MPipe implementation
#define MCU_CONFIG_MPIPEI2C             (NOT_AVAILABLE && !MCU_CONFIG_MPIPECDC)
#define MCU_CONFIG_MPIPEUART            (ENABLED && !MCU_CONFIG_MPIPECDC && !MCU_CONFIG_MPIPEI2C) 
#define MCU_CONFIG_MEMCPYDMA            ENABLED                             // MEMCPY DMA should be lower priority than MPIPE DMA
#define MCU_CONFIG_USB                  (MCU_CONFIG_MPIPECDC == ENABLED)
#define MCU_CONFIG_VOLTLEVEL            2   //(2-(MCU_CONFIG_MPIPEI2C==ENABLED))




/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.  For this configuration, 
  * Veelite is put into EEPROM.
  *
  * The STM32L0 uses 128 byte Flash pages and 2KB Flash sectors.  Therefore, it
  * is best to allocate the FS in Flash on 2KB boundaries because this is the 
  * resolution that can be write protected (or, as with FS, *NOT* write 
  * protected).  Best practice with STM32 chips is to put the FS at the back of
  * of the Flash space because this seems to work best with the debugger HW.
  */

// For building with non-default memory amounts
#ifndef SRAM_SIZE
#   define SRAM_SIZE            (20*1024)
#endif
#ifndef EEPROM_SIZE
#   define EEPROM_SIZE          (6*1024)
#endif
#ifndef FLASH_SIZE
#   define FLASH_SIZE           (192*1024)
#endif
#ifndef EEPROM_SAVE_SIZE
#   define EEPROM_SAVE_SIZE     (0)
#endif

// EEPROM LOCAL DATA
#define EEPROM_LOCAL_ADDR       (EEPROM_START_ADDR + EEPROM_SIZE - EEPROM_SAVE_SIZE)
#define EEPROM_LOCAL_U8         (ot_u8*)EEPROM_LOCAL_ADDR
#define EEPROM_LOCAL_U16        (ot_u16*)EEPROM_LOCAL_ADDR
#define EEPROM_LOCAL_U32        (ot_u32*)EEPROM_LOCAL_ADDR


// If not using EEPROM (using Flash) you need to coordinate some of these Flash Addresses
// with those entered into the Linker Script.  Use the vlsram linker script.
#if (defined(__NOEEPROM__) || defined(__VLSRAM__))
#   ifndef __VLSRAM__
#       define __VLSRAM__
#   endif
#   define FLASH_FS_END         (0x08018000)
#	define FLASH_NUM_PAGES      (FLASH_SIZE/FLASH_PAGE_SIZE)
#	define FLASH_FS_ALLOC       (4*1024) 							// 4KB FS memory
#	define FLASH_FS_ADDR        (FLASH_FS_END-FLASH_FS_ALLOC)	    // FS Flash Start Addr
#	define FLASH_FS_PAGES       (FLASH_FS_ALLOC / FLASH_PAGE_SIZE)
#	define FLASH_FS_FALLOWS     (0)			                        // No fallows

// Using EEPROM: Pages figure is irrelevant
#else
#	define FLASH_NUM_PAGES      (FLASH_SIZE/FLASH_PAGE_SIZE)
#	define FLASH_FS_ADDR        (EEPROM_START_ADDR)
#	define FLASH_FS_PAGES       0
#	define FLASH_FS_FALLOWS     0 
#	define FLASH_FS_ALLOC       (EEPROM_SIZE - EEPROM_SAVE_SIZE) 
#endif







/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  * 1. One button is supported on PC13.  The button must be attached by the 
  *    user.  It is active low.  Other buttons could be added on the unused
  *    pins, but only one is pre-defined.
  * 
  * 2. Two LEDs are defined on PC2, PC3, but the user must attach LEDs.
  *    <LI> TRIG1 (typically Orange LED) is on PC2.  It typically indicates 
  *         that the radio is on and in RX mode.</LI>
  *    <LI> TRIG2 (typically Green LED) is on PC3.  It typically indicates
  *         that the radio is on and in TX mode.</LI>
  *
  * 3. MPIPE can support UART TX/RX (no flow control) or I2C.  MPIPE supports
  *    a "break" mode, in both I2C and UART interfaces, where a break character 
  *    is transmitted by the client ahead of the packet in order to wakeup the 
  *    system.  This reduces power requirements.  Unfortunately, short-sighted
  *    design of the LoRa expansion board prevents the Break feature from 
  *    working on this board.
  *
  * 4. The Debug and Upload interface is SWD.  There is no serial bootloader
  *    capability at the time of writing.  
  *    (Editorial Note: A ST-Link-V2 has SWD and a low price of US$25.  
  *    It can work with OpenOCD, so the toolchain is still very cheap.)
  * 
  * 5. The SX127x SPI benefits from the highest speed clock up to 20 MHz.
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_PARAM(VAL)                BOARD_PARAM_##VAL

#define BOARD_FEATURE_MPIPE             ENABLED
#define BOARD_FEATURE_USBCONVERTER      BOARD_FEATURE_MPIPE         // Is UART connected via USB converter?
#define BOARD_FEATURE_MPIPE_BREAK       DISABLED                    // Send/receive leading break for wakeup
#define BOARD_FEATURE_MPIPE_DIRECT      (BOARD_FEATURE_MPIPE_BREAK != ENABLED) 

// MPIPE UART modes are deprecated.  Only Break-mode or Direct-Mode should be used.
//#define BOARD_FEATURE_MPIPE_CS          DISABLED                    // Chip-Select / DTR wakeup control
//#define BOARD_FEATURE_MPIPE_FLOWCTL     DISABLED                    // RTS/CTS style flow control 

#define BOARD_FEATURE_LFXTAL            ENABLED                                 // LF XTAL used as Clock source
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
#   warning "Nucleo board does not normally support XTAL-based USB.  Make sure you have a crystal mounted."
#   define BOARD_FEATURE_USBPLL         ENABLED
#elif MCU_CONFIG(USB)
#   undef BOARD_FEATURE_HFCRS
#   define BOARD_FEATURE_HFCRS          ENABLED
#   define BOARD_FEATURE_USBPLL         DISABLED
#else
#   define BOARD_FEATURE_USBPLL         DISABLED
#endif

// Number of Triggers/LEDs
#define BOARD_PARAM_TRIGS               2

// MPIPE speed: ignored with USB MPipe.  
// Actual speed will be closest supported bps.
#define BOARD_PARAM_MPIPEBAUD           115200

// Clock descriptions
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

// Clock divider specs: These are defined at FULL SPEED
#define BOARD_PARAM_AHBCLKDIV           1                       // AHB Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB2CLKDIV          1                       // APB2 Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB1CLKDIV          1                       // APB1 Clk = Main CLK / AHBCLKDIV

// SWD Interface: Nucleo used SWD in all cases, never JTAG.
// The SWO (Trace) pin is unfortunately consumed by the LoRa module
#define BOARD_SWDIO_PORT                GPIOA
#define BOARD_SWDIO_PINNUM              13
#define BOARD_SWDIO_PIN                 (1<<13)
#define BOARD_SWDCLK_PORT               GPIOA
#define BOARD_SWDCLK_PINNUM             14
#define BOARD_SWDCLK_PIN                (1<<14)
//#define BOARD_TRACESWO_PORTNUM          1
//#define BOARD_TRACESWO_PORT             GPIOB
//#define BOARD_TRACESWO_PINNUM           (1<<3)
//#define BOARD_TRACESWO_PIN              3

// SW1 is implemented on PC13, active low.
#define BOARD_SW1_PORTNUM               2                   // Port C
#define BOARD_SW1_PORT                  GPIOC
#define BOARD_SW1_PINNUM                13
#define BOARD_SW1_PIN                   (1<<BOARD_SW1_PINNUM)
#define BOARD_SW1_POLARITY              0
#define BOARD_SW1_PULLING               0

// TEST0, TEST1 pins are on PC4, PC5.
// They are not necessary, but Haystack uses them for manufacturing tests.
// They are usually outputs but they could be pull-up inputs also.
#define BOARD_TEST0_PORTNUM             2                   // Port C
#define BOARD_TEST0_PORT                GPIOC
#define BOARD_TEST0_PINNUM              4
#define BOARD_TEST0_PIN                 (1<<BOARD_TEST0_PINNUM)
#define BOARD_TEST0_POLARITY            0
#define BOARD_TEST0_PULLING             1
#define BOARD_TEST1_PORTNUM             2                   // Port C
#define BOARD_TEST1_PORT                GPIOC
#define BOARD_TEST1_PINNUM              5
#define BOARD_TEST1_PIN                 (1<<BOARD_TEST1_PINNUM)
#define BOARD_TEST1_POLARITY            0
#define BOARD_TEST1_PULLING             1

// LED interface is implemented on PC2 and PC3, which means you
// need to connect your own LEDS to these pins.
// Additionally, there is an optional Red-LED spec'ed on PC10
#define BOARD_LEDG_PORTNUM              2                   // Port C
#define BOARD_LEDG_PORT                 GPIOC
#define BOARD_LEDG_PINNUM               2
#define BOARD_LEDG_PIN                  (1<<BOARD_LEDG_PINNUM)
#define BOARD_LEDG_POLARITY             0
#define BOARD_LEDO_PORTNUM              2                   // Port C
#define BOARD_LEDO_PORT                 GPIOC
#define BOARD_LEDO_PINNUM               3
#define BOARD_LEDO_PIN                  (1<<BOARD_LEDO_PINNUM)
#define BOARD_LEDO_POLARITY             0
#define BOARD_LEDR_PORTNUM              2                   // Port C
#define BOARD_LEDR_PORT                 GPIOC
#define BOARD_LEDR_PINNUM               10
#define BOARD_LEDR_PIN                  (1<<BOARD_LEDR_PINNUM)
#define BOARD_LEDR_POLARITY             0

// LoRa Module SX127x Bus
#ifdef __USE_RADIO
#   define BOARD_RFGPIO_0PORTNUM        0        // "D2" : PA10
#   define BOARD_RFGPIO_0PORT           GPIOA
#   define BOARD_RFGPIO_0PINNUM         10
#   define BOARD_RFGPIO_0PIN            (1<<BOARD_RFGPIO_0PINNUM)
#   define BOARD_RFGPIO_1PORTNUM        1        // "D3" : PB3
#   define BOARD_RFGPIO_1PORT           GPIOB
#   define BOARD_RFGPIO_1PINNUM         3
#   define BOARD_RFGPIO_1PIN            (1<<BOARD_RFGPIO_1PINNUM)
#   define BOARD_RFGPIO_2PORTNUM        1        // "D4" : PB5
#   define BOARD_RFGPIO_2PORT           GPIOB
#   define BOARD_RFGPIO_2PINNUM         5
#   define BOARD_RFGPIO_2PIN            (1<<BOARD_RFGPIO_2PINNUM)
#   define BOARD_RFGPIO_3PORTNUM        1        // "D5" : PB4
#   define BOARD_RFGPIO_3PORT           GPIOB
#   define BOARD_RFGPIO_3PINNUM         4
#   define BOARD_RFGPIO_3PIN            (1<<BOARD_RFGPIO_3PINNUM)

// DIO4 is not connected in the hardware, and not required
// If you want to connect it, wire it to D6
//#   define BOARD_RFGPIO_4PORTNUM        1        // "D6" : PB10
//#   define BOARD_RFGPIO_4PORT           GPIOB
//#   define BOARD_RFGPIO_4PINNUM         10
//#   define BOARD_RFGPIO_4PIN            (1<<BOARD_RFGPIO_4PINNUM)

// DIO5 is not connected in the hardware, but if you can wire it to D7, 
// there's an improvement to efficiency & reliability
//#   define BOARD_RFGPIO_5PORTNUM        0        // "D7" : PA8
//#   define BOARD_RFGPIO_5PORT           GPIOA
//#   define BOARD_RFGPIO_5PINNUM         8
//#   define BOARD_RFGPIO_5PIN            (1<<BOARD_RFGPIO_5PINNUM)

#   define BOARD_RFCTL_RESETPORTNUM     0       // "A0" : PA0
#   define BOARD_RFCTL_RESETPORT        GPIOA
#   define BOARD_RFCTL_RESETPINNUM      0       
#   define BOARD_RFCTL_RESETPIN         (1<<BOARD_RFCTL_RESETPINNUM)
#   define BOARD_RFSPI_NSSPORTNUM       1       // "D10" : PB6
#   define BOARD_RFSPI_NSSPORT          GPIOB
#   define BOARD_RFSPI_NSSPINNUM        6       
#   define BOARD_RFSPI_NSSPIN           (1<<BOARD_RFSPI_NSSPINNUM)
#   define BOARD_RFSPI_ID               1       //SPI1
#   define BOARD_RFSPI_PORTNUM          0       //Port A
#   define BOARD_RFSPI_PORT             GPIOA
#   define BOARD_RFSPI_MOSIPINNUM       7       // "D11" : PA7
#   define BOARD_RFSPI_MISOPINNUM       6       // "D12" : PA6
#   define BOARD_RFSPI_SCLKPINNUM       5       // "D13" : PA5
#   define BOARD_RFSPI_MOSIPIN          (1<<BOARD_RFSPI_MOSIPINNUM)
#   define BOARD_RFSPI_MISOPIN          (1<<BOARD_RFSPI_MISOPINNUM)
#   define BOARD_RFSPI_SCLKPIN          (1<<BOARD_RFSPI_SCLKPINNUM)
#endif

// Main serial connections, used at least by MPIPE

// I2C on PB8/PB9 or "D15/D14"
#define BOARD_I2C_PORTNUM               1                       
#define BOARD_I2C_PORT                  GPIOB
#define BOARD_I2C_ID                    1
#define BOARD_I2C_SCLPINNUM             8
#define BOARD_I2C_SDAPINNUM             9
#define BOARD_I2C_SCLPIN                (1<<BOARD_I2C_SCLPINNUM)
#define BOARD_I2C_SDAPIN                (1<<BOARD_I2C_SDAPINNUM)

// UART on PA2/PA3 or "D1/D0"
#define BOARD_UART_PORTNUM              0
#define BOARD_UART_PORT                 GPIOA
#define BOARD_UART_ID                   2
#define BOARD_UART_TXPINNUM             2
#define BOARD_UART_RXPINNUM             3
#define BOARD_UART_TXPIN                (1<<BOARD_UART_TXPINNUM)
#define BOARD_UART_RXPIN                (1<<BOARD_UART_RXPINNUM)

// USB is always on PA11/A12
#define BOARD_USB_PORTNUM               0
#define BOARD_USB_PORT                  GPIOA
#define BOARD_USB_DMPINNUM              11
#define BOARD_USB_DPPINNUM              12
#define BOARD_USB_DMPIN                 (1<<BOARD_USB_DMPINNUM)
#define BOARD_USB_DPPIN                 (1<<BOARD_USB_DPPINNUM)

// ADC Analog inputs on PA0,PA1,PA4,PB0,PC1,PC0 or "A0,A1,A2,A3,A4,A5"
#define BOARD_ADC_PINS                  6
#define BOARD_ADC_PORTS                 3
#define BOARD_ADC_0PORTNUM              0
#define BOARD_ADC_0PORT                 GPIOA
#define BOARD_ADC_0CHAN                 0
#define BOARD_ADC_0PINNUM               0
#define BOARD_ADC_0PIN                  (1<<BOARD_ADC_0PINNUM)
#define BOARD_ADC_1PORTNUM              0
#define BOARD_ADC_1PORT                 GPIOA
#define BOARD_ADC_1CHAN                 1
#define BOARD_ADC_1PINNUM               1
#define BOARD_ADC_1PIN                  (1<<BOARD_ADC_1PINNUM)
#define BOARD_ADC_2PORTNUM              0
#define BOARD_ADC_2PORT                 GPIOA
#define BOARD_ADC_2CHAN                 4
#define BOARD_ADC_2PINNUM               4
#define BOARD_ADC_2PIN                  (1<<BOARD_ADC_2PINNUM)
#define BOARD_ADC_3PORTNUM              1
#define BOARD_ADC_3PORT                 GPIOB
#define BOARD_ADC_3CHAN                 8
#define BOARD_ADC_3PINNUM               0
#define BOARD_ADC_3PIN                  (1<<BOARD_ADC_3PINNUM)
#define BOARD_ADC_4PORTNUM              2
#define BOARD_ADC_4PORT                 GPIOC
#define BOARD_ADC_4CHAN                 1
#define BOARD_ADC_4PINNUM               1
#define BOARD_ADC_4PIN                  (1<<BOARD_ADC_4PINNUM)
#define BOARD_ADC_5PORTNUM              2
#define BOARD_ADC_5PORT                 GPIOC
#define BOARD_ADC_5CHAN                 0
#define BOARD_ADC_5PINNUM               0
#define BOARD_ADC_5PIN                  (1<<BOARD_ADC_5PINNUM)



// Timer IO: these are multiplexed onto the IOBUS
///@todo convert these to PWM Nomenclature
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
#define BOARD_TIMB_PORTNUM              1
#define BOARD_TIMB_PORT                 GPIOB
#define BOARD_TIMB_ID                   4
#define BOARD_TIMB                      TIM4
#define BOARD_TIMB_CHANS                4
#define BOARD_TIMB_1CHAN                1
#define BOARD_TIMB_2CHAN                2
#define BOARD_TIMB_3CHAN                3
#define BOARD_TIMB_4CHAN                4
#define BOARD_TIMB_1PINNUM              6
#define BOARD_TIMB_2PINNUM              7
#define BOARD_TIMB_3PINNUM              8
#define BOARD_TIMB_4PINNUM              9
#define BOARD_TIMB_1PIN                 (1<<BOARD_TIMB_1PINNUM)
#define BOARD_TIMB_2PIN                 (1<<BOARD_TIMB_2PINNUM)
#define BOARD_TIMB_3PIN                 (1<<BOARD_TIMB_3PINNUM)
#define BOARD_TIMB_4PIN                 (1<<BOARD_TIMB_4PINNUM)
#define BOARD_TIM
#define BOARD_TIMC_PORTNUM              1
#define BOARD_TIMC_PORT                 GPIOB
#define BOARD_TIMC_ID                   10
#define BOARD_TIMC                      TIM10
#define BOARD_TIMC_CHANS                1
#define BOARD_TIMC_1CHAN                1
#define BOARD_TIMC_1PINNUM              8
#define BOARD_TIMC_1PIN                 (1<<BOARD_TIMC_1PINNUM)
#define BOARD_TIMD_PORTNUM              1
#define BOARD_TIMD_PORT                 GPIOB
#define BOARD_TIMD_ID                   11
#define BOARD_TIMD                      TIM11
#define BOARD_TIMD_CHANS                1
#define BOARD_TIMD_1CHAN                1
#define BOARD_TIMD_1PINNUM              9
#define BOARD_TIMD_1PIN                 (1<<BOARD_TIMD_1PINNUM)




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
    //RCC->APB1SMENR = (RCC_APB1ENR_LPTIM1EN | RCC_APB1ENR_PWREN);  // Enabled by default
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
///      - Radio EXTIs: {GPIO-0, -1, -3} : {PA10, PB3, PB4}
#ifdef __USE_RADIO
#   define _EXTICR3_PORTNUM     BOARD_RFGPIO_1PORTNUM
#   define _EXTICR4_PORTNUM     BOARD_RFGPIO_3PORTNUM
#   define _EXTICR5_PORTNUM     BOARD_RFGPIO_2PORTNUM
#   define _EXTICR10_PORTNUM    BOARD_RFGPIO_0PORTNUM
#else
#   define _EXTICR3_PORTNUM     0
#   define _EXTICR4_PORTNUM     0
#   define _EXTICR5_PORTNUM     0
#   define _EXTICR10_PORTNUM    0
#endif
static inline void BOARD_EXTI_STARTUP(void) {
    // EXTI0-3: A0, A1, B2, RFGPIO-1 (B3)
    SYSCFG->EXTICR[0]   = (0 << 0) \
                        | (0 << 4) \
                        | (1 << 8) \
                        | (_EXTICR3_PORTNUM << 12);
    
    // EXTI4-7: RFGPIO-3 (B4), RFGPIO-2 (B5), B6, C7
    SYSCFG->EXTICR[1]   = (_EXTICR4_PORTNUM << 0) \
                        | (_EXTICR5_PORTNUM << 4) \
                        | (1 << 8) \
                        | (2 << 12);
                        
    // EXTI8-11: A8, A9, RFGPIO-0 (A10), A11
    SYSCFG->EXTICR[2]   = (0 << 0) \
                        | (0 << 4) \
                        | (_EXTICR10_PORTNUM << 8) \
                        | (0 << 12);

    // EXTI12-15: A12, SW1 (C13), B14, A15
    SYSCFG->EXTICR[3]   = (0 << 0) \
                        | (BOARD_SW1_PORTNUM << 4) \
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
    // - A0:  Radio Reset, which starts as a floating input and the driver later sorts-out
    // - A1:  Analog Input
    // - A2:  UART-TX, which is ALT push-pull output
    // - A3:  UART-RX, which is ALT push-pull input
    // - A4:  Analog Input
    // - A5:7 are SPI bus, set to ALT.
    // - A8:  This is where Radio GPIO5 goes, if enabled.  Otherwise, Unused Analog-In
    // - A9:  Unused Analog-In
    // - A10: Radio GPIO0, Hi-Z input
    // - A11:12 are inputs which are not available on the Arduino pinout (USB)
    // - A13:14 are SWD, set to ALT
    // - A15: Unused Analog-In
    
#   if defined(BOARD_RFGPIO_5PIN)
#       define _GPIO_MODER_PA8  GPIO_MODER_IN
#   else
#       define _GPIO_MODER_PA8  GPIO_MODER_ANALOG
#   endif
    GPIOA->MODER    = (GPIO_MODER_IN    << (0*2)) \
                    | (GPIO_MODER_ANALOG << (1*2)) \
                    | (GPIO_MODER_ALT    << (2*2)) \
                    | (GPIO_MODER_ALT    << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_ALT    << (5*2)) \
                    | (GPIO_MODER_ALT    << (6*2)) \
                    | (GPIO_MODER_ALT    << (7*2)) \
                    | (_GPIO_MODER_PA8   << (8*2)) \
                    | (GPIO_MODER_ANALOG << (9*2)) \
                    | (GPIO_MODER_IN     << (10*2)) \
                    | (GPIO_MODER_IN     << (11*2)) \
                    | (GPIO_MODER_IN     << (12*2)) \
                    | (GPIO_MODER_ALT    << (13*2)) \
                    | (GPIO_MODER_ALT    << (14*2)) \
                    | (GPIO_MODER_ANALOG << (15*2));

    GPIOA->OTYPER   = 0;
    
    GPIOA->OSPEEDR  = (GPIO_OSPEEDR_2MHz << (2*2)) \
                    | (GPIO_OSPEEDR_2MHz << (3*2)) \
                    | (GPIO_OSPEEDR_10MHz << (5*2)) \
                    | (GPIO_OSPEEDR_10MHz << (6*2)) \
                    | (GPIO_OSPEEDR_10MHz << (7*2)) \
                    | (GPIO_OSPEEDR_40MHz << (13*2)) \
                    | (GPIO_OSPEEDR_40MHz << (14*2));
    
    GPIOA->PUPDR    = (1 << (3*2)) \
                    | (1 << (13*2)) | (2 << (14*2));
    

#   ifndef __USE_RADIO
    GPIOA->AFR[0]   = (4 << (BOARD_UART_TXPINNUM*4)) \
                    | (4 << (BOARD_UART_RXPINNUM*4));
#   else
    GPIOA->AFR[0]   = (4 << (BOARD_UART_TXPINNUM*4)) \
                    | (4 << (BOARD_UART_RXPINNUM*4)) \
                    | (0 << ((BOARD_RFSPI_MOSIPINNUM)*4)) \
                    | (0 << ((BOARD_RFSPI_MISOPINNUM)*4)) \
                    | (0 << ((BOARD_RFSPI_SCLKPINNUM)*4));
#   endif

    /// If USB is used, set ALT.
    ///@todo validate this ALT implementation.
#   if (MCU_CONFIG(USB))
    GPIOA->AFR[1]   = (10 << ((BOARD_USB_DMPINNUM-8)*4)) \
                    | (10 << ((BOARD_USB_DPPINNUM-8)*4));
#   endif

    /// Configure Port B IO.
    /// Port B is used for external (module) IO.
    // - B0:  Analog Input
    // - B1:2 are unused and set to Analog mode
    // - B3:  Radio GPIO input, else unused Analog
    // - B4:  Radio GPIO input, else unused Analog
    // - B5:  Radio GPIO input, else unused Analog
    // - B6:  Radio SPI-NSS, an output
    // - B7:  unused and set to Analog mode
    // - B8:9 are setup for I2C bus: They start as output with Open Drain
    // - B10: is a PWM output, set to Analog until configured
    // - B11:15 are unused and set to Analog mode
#   if defined(__USE_RADIO)
    GPIOB->BSRR     = BOARD_RFSPI_NSSPIN;
#       define _GPIO_MODER_PB3  GPIO_MODER_IN
#       define _GPIO_MODER_PB4  GPIO_MODER_IN
#       define _GPIO_MODER_PB5  GPIO_MODER_IN
#       define _GPIO_MODER_PB6  GPIO_MODER_OUT
#   else
#       define _GPIO_MODER_PB3  GPIO_MODER_ANALOG
#       define _GPIO_MODER_PB4  GPIO_MODER_ANALOG
#       define _GPIO_MODER_PB5  GPIO_MODER_ANALOG
#       define _GPIO_MODER_PB6  GPIO_MODER_ANALOG
#   endif
    
    GPIOB->OTYPER   = (1 << (8)) | (1 << (9));
    GPIOB->BSRR     = (1 << (8)) | (1 << (9));
    GPIOB->MODER    = (GPIO_MODER_ANALOG << (0*2)) \
                    | (GPIO_MODER_ANALOG << (1*2)) \
                    | (GPIO_MODER_ANALOG << (2*2)) \
                    | (_GPIO_MODER_PB3 << (3*2)) \
                    | (_GPIO_MODER_PB4 << (4*2)) \
                    | (_GPIO_MODER_PB5 << (5*2)) \
                    | (_GPIO_MODER_PB6 << (6*2)) \
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
    // - C0:1   are analog inputs
    // - C2:3   LED0 and LED1 Output Triggers: Push-Pull output or Open Drain depending on Polarity
    // - C4:5   TEST0 and TEST1 outputs
    // - C6:9   are unused and set to analog
    // - C10    LED2 (optional red): Push-Pull output or Open Drain depending on Polarity
    // - C11:12 are unused and set to analog
    // - C13 is the user button, set to input HiZ
    // - C14:15 are 32kHz crystal driving, they are set in a particular way
#   if (BOARD_LEDG_POLARITY == 0)
#       define _GPIO_OTYPER_PC2     (1<<2)
#       define _GPIO_ODR_PC2        (1<<2)
#   else
#       define _GPIO_OTYPER_PC2     0
#       define _GPIO_ODR_PC2        0
#   endif
#   if (BOARD_LEDO_POLARITY == 0)
#       define _GPIO_OTYPER_PC3     (1<<3)
#       define _GPIO_ODR_PC3        (1<<3)
#   else
#       define _GPIO_OTYPER_PC3     0
#       define _GPIO_ODR_PC3        0
#   endif
#   if (BOARD_LEDR_POLARITY == 0)
#       define _GPIO_OTYPER_PC10    (1<<10)
#       define _GPIO_ODR_PC10       (1<<10)
#   else
#       define _GPIO_OTYPER_PC10    0
#       define _GPIO_ODR_PC10       0
#   endif
#   ifdef __USE_RADIO
#       define _GPIO_ODR_RFRESET    BOARD_RFCTL_RESETPIN
#   else
#       define _GPIO_ODR_RFRESET    0
#   endif

    GPIOA->OTYPER   = (_GPIO_OTYPER_PC2 | _GPIO_OTYPER_PC3 | _GPIO_OTYPER_PC10);
    GPIOC->BSRR     = (_GPIO_ODR_PC2 | _GPIO_ODR_PC3 | _GPIO_ODR_PC10 | _GPIO_ODR_RFRESET);
    

    GPIOC->MODER    = (GPIO_MODER_ANALOG << (0*2)) \
                    | (GPIO_MODER_ANALOG << (1*2)) \
                    | (GPIO_MODER_OUT    << (2*2)) \
                    | (GPIO_MODER_OUT    << (3*2)) \
                    | (GPIO_MODER_OUT    << (4*2)) \
                    | (GPIO_MODER_OUT    << (5*2)) \
                    | (GPIO_MODER_ANALOG << (6*2)) \
                    | (GPIO_MODER_ANALOG << (7*2)) \
                    | (GPIO_MODER_ANALOG << (8*2)) \
                    | (GPIO_MODER_ANALOG << (9*2)) \
                    | (GPIO_MODER_OUT    << (10*2)) \
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
    
    // Disable GPIOH, it is never altered after this.
    RCC->IOPENR = _IOPENR_RUNTIME;
}


/*
static inline void BOARD_OPEN_FLASH(void* start, void* end) {
/// STM32L0 reserves flash on 4KB boundaries
#   define _F_LAST ((FLASH_SIZE-1) >> 12)
    ot_u32 a, b;
    ot_u32 bmask1, bmask2;
    
    // 4KB Boundaries: a to b
    a = ((ot_u32)start - 0x08000000) >> 12;
    b = ((ot_u32)last - 0x08000000) >> 12;
    
    // Make sure a, b are <= _F_LAST. Alter accordingly
    if (a > _F_LAST) a = _F_LAST;
    if (b > _F_LAST) b = _F_LAST;
    
    // Create bitmask from a and b bit positions
    switch ( ((a>=32)<<1) | (b>=32) ) {
        // a < 32, b < 32
        case 0: bmask1  = 1 << a;
                bmask1 |= (bmask1-1);
                bmask1 ^= (1<<b) - 1;
                bmask2  = 0;
                break;
                
        // a < 32, b >= 32
        case 1: bmask1  = 1 << a;
                bmask1 |= (bmask1-1);
                bmask2  = 1 << b;
                bmask2 |= ~(bmask2-1);
                break;
        
        // a >= 32, b < 32 -- implausible
        case 2: return;
        
        // a >= 32, b >= 32
        case 3: bmask1  = 0;
                bmask2  = 1 << a;
                bmask2 |= (bmask2-1);
                bmask2 ^= (1<<b) - 1;
                break;
        
    }
    
    // Apply bitmasks to WRPROT registers
    
    //FLASH->WRPROT1  = __REV(bmask1);
    //FLASH->WRPROT2  = __REV(bmask2);
}
*/


static inline void BOARD_RFSPI_CLKON(void) {
#ifdef __USE_RADIO
    ot_u32 spi_moder;
    spi_moder   = BOARD_RFSPI_PORT->MODER;
    spi_moder  &= ~((3 << (BOARD_RFSPI_SCLKPINNUM*2)) \
                  | (3 << (BOARD_RFSPI_MISOPINNUM*2)) \
                  | (3 << (BOARD_RFSPI_MOSIPINNUM*2)) );
    spi_moder  |= (GPIO_MODER_ALT << (BOARD_RFSPI_SCLKPINNUM*2)) \
                | (GPIO_MODER_ALT << (BOARD_RFSPI_MISOPINNUM*2)) \
                | (GPIO_MODER_ALT << (BOARD_RFSPI_MOSIPINNUM*2));
    
    BOARD_RFSPI_PORT->MODER = spi_moder;
#endif
}

static inline void BOARD_RFSPI_CLKOFF(void) {
#ifdef __USE_RADIO
    ot_u32 spi_moder;
    spi_moder   = BOARD_RFSPI_PORT->MODER;
    spi_moder  &= ~((3 << (BOARD_RFSPI_SCLKPINNUM*2)) \
                  | (3 << (BOARD_RFSPI_MISOPINNUM*2)) \
                  | (3 << (BOARD_RFSPI_MOSIPINNUM*2)) );
    spi_moder  |= (GPIO_MODER_OUT << (BOARD_RFSPI_SCLKPINNUM*2)) \
                | (GPIO_MODER_IN << (BOARD_RFSPI_MISOPINNUM*2)) \
                | (GPIO_MODER_OUT << (BOARD_RFSPI_MOSIPINNUM*2));
    
    BOARD_RFSPI_PORT->MODER = spi_moder;
#endif
}



#include <platform/timers.h>    // for systim_stop_clocker()
static inline void BOARD_STOP(ot_int code) {
/// code comes from sys_sig_powerdown, but it is usually 0-3.
/// For all STM32L devices, 3 is full-idle and 2 is radio-active-idle.  
/// Those are the only modes that should call this inline function.

    static const ot_u16 stop_flags[2] = {  
        (PWR_CR_LPSDSR | PWR_CR_CSBF), (PWR_CR_LPSDSR | PWR_CR_FWU | PWR_CR_ULP | PWR_CR_CSBF) };
    
#   if defined(__RELEASE__)
    static const ot_u32 rcc_flags[2] = {
        _GPIOCLK_N, 0 };
#	endif 
    
    //static const ot_u32 b_moder[2] = {
    //    0xFFFFFC0, 0xFFFFFFFF };
    
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
    
    /*BOARD_USB_PORT->MODER  |= (2 << (BOARD_USB_DMPINNUM*2)) \
                              | (2 << (BOARD_USB_DPPINNUM*2)); */
}

static inline void BOARD_USB_PORTDISABLE(void) {
    USB->BCDR &= ~(ot_u32)USB_BCDR_DPPU;
    
    /*BOARD_USB_PORT->MODER  &= ~( (3 << (BOARD_USB_DMPINNUM*2)) \
                                 | (3 << (BOARD_USB_DPPINNUM*2)) ); */
}


   

///@note BOARD Macros for Radio module interrupt vectoring.  Connect these to
///      the radio interface driver you are using.  Check the schematic of your
///      board to see where the Radio IRQ lines are routed.  Also check the 
///      radio interface header documentation (it's really quite simple). 
///      These Macros will get called in the universal EXTI interrupt handler,
///      typically implemented in platform_isr_STM32L.c
#define BOARD_RADIO_EXTI0_ISR(); 
#define BOARD_RADIO_EXTI1_ISR();   
#define BOARD_RADIO_EXTI2_ISR();
#define BOARD_RADIO_EXTI3_ISR()     sx127x_irq1_isr()
#define BOARD_RADIO_EXTI4_ISR()     sx127x_irq3_isr()
#define BOARD_RADIO_EXTI5_ISR();
#define BOARD_RADIO_EXTI6_ISR();     
#define BOARD_RADIO_EXTI7_ISR();     
#define BOARD_RADIO_EXTI8_ISR();
#define BOARD_RADIO_EXTI9_ISR();
#define BOARD_RADIO_EXTI10_ISR()    sx127x_irq0_isr()    
#define BOARD_RADIO_EXTI11_ISR();
#define BOARD_RADIO_EXTI12_ISR();
#define BOARD_RADIO_EXTI13_ISR();
#define BOARD_RADIO_EXTI14_ISR();
#define BOARD_RADIO_EXTI15_ISR();


///@todo Create a more intelligent setup that knows how to use the UART, even
///      though for this board UART is not generally the MPipe
///@note BOARD Macros for Com module interrupt vectoring.  Connect these to
///      the Com interface driver you are using.  Check the schematic of your
///      board to see where the Com IRQ lines are routed. 
#if (MCU_CONFIG(MPIPEUART) && OT_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE_BREAK))
#   define _UART_RXSYNC_ISR()       mpipe_rxsync_isr()
#else
#   define _UART_RXSYNC_ISR()       ;
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
#define OT_GPTIM_SHIFT      0
#define OT_GPTIM_OVERSAMPLE 1
#define OT_GPTIM_RES        (1024 << OT_GPTIM_SHIFT)    //1024
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
#define OT_TRIG3_PORT       BOARD_LEDR_PORT
#define OT_TRIG3_PINNUM     BOARD_LEDR_PINNUM
#define OT_TRIG3_PIN        BOARD_LEDR_PIN
#define OT_TRIG3_POLARITY   BOARD_LEDR_POLARITY

#define OT_TRIG(NUM, SET)   OT_TRIG##NUM##_##SET##()

#if (OT_TRIG1_POLARITY != 0)
#   define OT_TRIG1_ON()    OT_TRIG1_PORT->BSRR  = OT_TRIG1_PIN;
#   define OT_TRIG1_OFF()   OT_TRIG1_PORT->BRR   = OT_TRIG1_PIN;
#   define OT_TRIG1_TOG()   OT_TRIG1_PORT->ODR  ^= OT_TRIG1_PIN;
#else 
#   define OT_TRIG1_ON()    OT_TRIG1_PORT->BRR   = OT_TRIG1_PIN;
#   define OT_TRIG1_OFF()   OT_TRIG1_PORT->BSRR  = OT_TRIG1_PIN;
#   define OT_TRIG1_TOG()   OT_TRIG1_PORT->ODR  ^= OT_TRIG1_PIN;
#endif

#if (OT_TRIG2_POLARITY != 0)
#   define OT_TRIG2_ON()    OT_TRIG2_PORT->BSRR  = OT_TRIG2_PIN;
#   define OT_TRIG2_OFF()   OT_TRIG2_PORT->BRR   = OT_TRIG2_PIN;
#   define OT_TRIG2_TOG()   OT_TRIG2_PORT->ODR  ^= OT_TRIG2_PIN;
#else 
#   define OT_TRIG2_ON()    OT_TRIG2_PORT->BRR   = OT_TRIG2_PIN;
#   define OT_TRIG2_OFF()   OT_TRIG2_PORT->BSRR  = OT_TRIG2_PIN;
#   define OT_TRIG2_TOG()   OT_TRIG2_PORT->ODR  ^= OT_TRIG2_PIN;
#endif

#if (OT_TRIG3_POLARITY != 0)
#   define OT_TRIG3_ON()    OT_TRIG3_PORT->BSRR  = OT_TRIG3_PIN;
#   define OT_TRIG3_OFF()   OT_TRIG3_PORT->BRR   = OT_TRIG3_PIN;
#   define OT_TRIG3_TOG()   OT_TRIG3_PORT->ODR  ^= OT_TRIG3_PIN;
#else 
#   define OT_TRIG3_ON()    OT_TRIG3_PORT->BRR   = OT_TRIG3_PIN;
#   define OT_TRIG3_OFF()   OT_TRIG3_PORT->BSRR  = OT_TRIG3_PIN;
#   define OT_TRIG3_TOG()   OT_TRIG3_PORT->ODR  ^= OT_TRIG3_PIN;
#endif

static inline void BOARD_led1_on(void)      { OT_TRIG1_ON(); }
static inline void BOARD_led1_off(void)     { OT_TRIG1_OFF(); }
static inline void BOARD_led1_toggle(void)  { OT_TRIG1_TOG(); }
static inline void BOARD_led2_on(void)      { OT_TRIG2_ON(); }
static inline void BOARD_led2_off(void)     { OT_TRIG2_OFF(); }
static inline void BOARD_led2_toggle(void)  { OT_TRIG2_TOG(); }
static inline void BOARD_led3_on(void)      { OT_TRIG3_ON(); }
static inline void BOARD_led3_off(void)     { OT_TRIG3_OFF(); }
static inline void BOARD_led3_toggle(void)  { OT_TRIG3_TOG(); }


#ifndef __ISR_EXTI13
#   define __ISR_EXTI13
#endif
#define OT_SWITCH1_ISR      platform_isr_exti13
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
#if (OT_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE))
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
#   if (MPIPE_UART_ID != 2)
#       error "MPIPE UART must be on USART2 for this board."
#   endif
//#   ifndef __ISR_USART2
//#       define __ISR_USART2
//#   endif
#   define MPIPE_UART           USART2
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
#define RADIO_SPICS_PORT            BOARD_RFSPI_NSSPORT
#define RADIO_SPIMOSI_PIN           BOARD_RFSPI_MOSIPIN
#define RADIO_SPIMISO_PIN           BOARD_RFSPI_MISOPIN
#define RADIO_SPICLK_PIN            BOARD_RFSPI_SCLKPIN
#define RADIO_SPICS_PIN             BOARD_RFSPI_NSSPIN

#define RADIO_IRQ0_SRCPORT          BOARD_RFGPIO_0PORTNUM
#define RADIO_IRQ1_SRCPORT          BOARD_RFGPIO_1PORTNUM
#define RADIO_IRQ2_SRCPORT          BOARD_RFGPIO_2PORTNUM
#define RADIO_IRQ3_SRCPORT          BOARD_RFGPIO_3PORTNUM
//#define RADIO_IRQ4_SRCPORT          BOARD_RFGPIO_4PORTNUM
//#define RADIO_IRQ5_SRCPORT          BOARD_RFGPIO_5PORTNUM
#define RADIO_IRQ0_SRCLINE          BOARD_RFGPIO_0PINNUM
#define RADIO_IRQ1_SRCLINE          BOARD_RFGPIO_1PINNUM
#define RADIO_IRQ2_SRCLINE          -1
#define RADIO_IRQ3_SRCLINE          BOARD_RFGPIO_3PINNUM
//#define RADIO_IRQ4_SRCLINE          -1
//#define RADIO_IRQ5_SRCLINE          -1

#define RADIO_RESET_PORT            BOARD_RFCTL_RESETPORT
#define RADIO_IRQ0_PORT             BOARD_RFGPIO_0PORT
#define RADIO_IRQ1_PORT             BOARD_RFGPIO_1PORT
#define RADIO_IRQ2_PORT             BOARD_RFGPIO_2PORT
#define RADIO_IRQ3_PORT             BOARD_RFGPIO_3PORT
//#define RADIO_IRQ4_PORT             BOARD_RFGPIO_4PORT
//#define RADIO_IRQ5_PORT             BOARD_RFGPIO_5PORT
#define RADIO_RESET_PIN             BOARD_RFCTL_RESETPIN
#define RADIO_IRQ0_PIN              BOARD_RFGPIO_0PIN
#define RADIO_IRQ1_PIN              BOARD_RFGPIO_1PIN
#define RADIO_IRQ2_PIN              BOARD_RFGPIO_2PIN
#define RADIO_IRQ3_PIN              BOARD_RFGPIO_3PIN
//#define RADIO_IRQ4_PIN              BOARD_RFGPIO_4PIN
//#define RADIO_IRQ5_PIN              BOARD_RFGPIO_5PIN
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
  * On this module, all EXTIs not being used by integrated peripherals are 
  * available to the user app or system.
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





/** Trailing Header Includes <BR>
  * ========================================================================<BR>
  * These must occur AFTER all the above has been defined
  */
#ifdef __USE_RADIO
#	include <io/sx127x/interface.h>
#endif

#endif
