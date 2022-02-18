/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /board/stm32wl55xx/nucleo_wl55.h
  * @author     JP Norair
  * @version    R100
  * @date       19 Jan 2022
  * @brief      Board Configuration for ST Nucleo WL55
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
  

#ifndef __nucleo_wl55_H
#define __nucleo_wl55_H

/// MCU definition for the board must be the first thing.
#define __STM32WL55JC__

#include <app/app_config.h>
#include <platform/hw/STM32WLxx_config.h>
#include <platform/interrupts.h>

/// Right here it's stipulated what features CPU1 gets to control.
#ifndef MCU_PARAM_CPU1NULL
#   define MCU_PARAM_CPU1NULL   1
#endif
#ifndef MCU_PARAM_CPU2PWR
#   define MCU_PARAM_CPU2PWR    (1 && MCU_PARAM_CPU1NULL)
#endif


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
#       include <io/radio_null/config.h>
#   else
#       define __LORA__
#		define __STM32WL_LORA__
#		define __STM32WL_22dBm__
#       define __STM32WL_15dBm__
#       include <io/stm32wl_lora/config.h>
#   endif
#endif



/** Additional RF Front End Parameters and Settings <BR>
  * ========================================================================<BR>
  * We assume you are using the included antenna and matching circuit, which 
  * has a 50% efficiency (-3dB) with 1dB insertion loss
  */
#ifdef __USE_RADIO
#   ifndef RF_PARAM_BAND
#   define RF_PARAM_BAND    915
#   endif
#   define RF_HDB_ATTEN     4                //Half dB attenuation (units = 0.5dB), used to scale TX power
#   define RF_HDB_RXATTEN   4
#   define RF_RSSI_OFFSET   RF_HDB_RXATTEN   //Offset applied to RSSI calculation
#endif



/** MCU Configuration settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the STM32WL variants on this board/build
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
#define MCU_CONFIG_MPIPEI2C             (NOT_AVAILABLE && !MCU_CONFIG_MPIPECDC)
#define MCU_CONFIG_MPIPEUART            (ENABLED && !MCU_CONFIG_MPIPEI2C)
#define MCU_CONFIG_MEMCPYDMA            ENABLED                             // MEMCPY DMA should be lower priority than MPIPE DMA
#define MCU_CONFIG_USB                  (MCU_CONFIG_MPIPECDC == ENABLED)
#define MCU_CONFIG_VOLTLEVEL            2




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
#ifndef SRAM_AVAILABLE
#   define SRAM_AVAILABLE            (16*1024)
#endif
#ifndef EEPROM_AVAILABLE
#   define EEPROM_AVAILABLE          (0*1024)
#endif
#ifndef FLASH_AVAILABLE
#   define FLASH_AVAILABLE           (64*1024)
#endif
#ifndef EEPROM_SAVE_SIZE
#   define EEPROM_SAVE_SIZE     (0)
#endif


// If not using EEPROM (using Flash) you need to coordinate some of these Flash Addresses
// with those entered into the Linker Script.  Use the vlsram linker script.
#ifndef __VLSRAM__
#   define __VLSRAM__
#endif
#define FLASH_FS_END        (0x08040000)
#define FLASH_NUM_PAGES     (FLASH_AVAILABLE/FLASH_PAGE_SIZE)
#define FLASH_FS_ALLOC      (4*1024) 							// 4KB FS memory
#define FLASH_FS_ADDR       (FLASH_FS_END-FLASH_FS_ALLOC)	    // FS Flash Start Addr
#define FLASH_FS_PAGE0      ((FLASH_FS_ADDR - FLASH_BASE) / FLASH_PAGE_SIZE)
#define FLASH_FS_PAGES      (FLASH_FS_ALLOC / FLASH_PAGE_SIZE)
#define FLASH_FS_FALLOWS    (0)			                        // No fallows







/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  * 1. One button is supported on PB2.  It is the "User" button that comes with
  *    the discovery board.
  * 
  * 2. There are many LEDs available:
  *    <LI> LD1, Green, PB5.  It is used as TRIG2 to show RF TX active. </LI>
  *    <LI> LD2, Red, PA5.  It is used as TRIG1 to show RF RX active. </LI>
  *    <LI> LD3, Blue, PB6.  It is unused. </LI>
  *    <LI> LD4, Red, PB7.  It is unused. </LI>
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
#define BOARD_FEATURE_MPIPE_BREAK       ENABLED                    // Send/receive leading break for wakeup
#define BOARD_FEATURE_MPIPE_DIRECT      (BOARD_FEATURE_MPIPE_BREAK != ENABLED) 

#define BOARD_FEATURE_I2C               DISABLED

// MPIPE UART modes are deprecated.  Only Break-mode or Direct-Mode should be used.
//#define BOARD_FEATURE_MPIPE_CS          DISABLED                    // Chip-Select / DTR wakeup control
//#define BOARD_FEATURE_MPIPE_FLOWCTL     DISABLED                    // RTS/CTS style flow control 

#ifndef BOARD_FEATURE_LFXTAL
#define BOARD_FEATURE_LFXTAL            ENABLED                                 // LF XTAL used as Clock source
#endif
#ifndef BOARD_FEATURE_HFXTAL
#define BOARD_FEATURE_HFXTAL            DISABLED                                // HF XTAL used as Clock source
#endif
#ifndef BOARD_FEATURE_HFBYPASS
#define BOARD_FEATURE_HFBYPASS          DISABLED                                // Use an externally driven oscillator
#endif
#ifndef BOARD_FEATURE_RFXTAL
#define BOARD_FEATURE_RFXTAL            ENABLED                                 // XTAL for RF chipset
#endif
#ifndef BOARD_FEATURE_RFXTALOUT
#define BOARD_FEATURE_RFXTALOUT         DISABLED
#endif
#ifndef BOARD_FEATURE_PLL
#define BOARD_FEATURE_PLL               DISABLED
#endif
#ifndef BOARD_FEATURE_STDSPEED
#define BOARD_FEATURE_STDSPEED          (MCU_CONFIG_MULTISPEED == ENABLED)
#endif
#ifndef BOARD_FEATURE_FULLSPEED
#define BOARD_FEATURE_FULLSPEED         ENABLED
#endif
#ifndef BOARD_FEATURE_FULLXTAL
#define BOARD_FEATURE_FULLXTAL          DISABLED
#endif
#ifndef BOARD_FEATURE_FLANKSPEED
#define BOARD_FEATURE_FLANKSPEED        DISABLED
#endif
#ifndef BOARD_FEATURE_FLANKXTAL
#define BOARD_FEATURE_FLANKXTAL         DISABLED
#endif

#if !defined(BOARD_FEATURE_RF_PABOOST)
#	if !defined(__SX127x_PABOOST__)
#	define BOARD_FEATURE_RF_PABOOST		DISABLED
#	else
#	define BOARD_FEATURE_RF_PABOOST		ENABLED
#	endif
#endif

// Number of Triggers/LEDs
#ifndef BOARD_PARAM_TRIGS
#   define BOARD_PARAM_TRIGS            3
#endif

// Number of Switches
#ifndef BOARD_PARAM_SWITCHES
#   define BOARD_PARAM_SWITCHES         3
#endif

// Number of SW-only EXTIs
// These are used on EXTI lines in place of edge detectors
#ifndef BOARD_PARAM_SWEXTIS
#   define BOARD_PARAM_SWEXTIS          1
#endif

// MPIPE speed: ignored with USB MPipe.  
// Actual speed will be closest supported bps.
#define BOARD_PARAM_MPIPEBAUD           115200

// Clock descriptions
#define BOARD_PARAM_LFHz                32768
#define BOARD_PARAM_LFtol               0.00002

#define BOARD_PARAM_MFHz                48000000
#define BOARD_PARAM_MFmult              1                       // Main CLK = HFHz * (MFmult/MFdiv)
#define BOARD_PARAM_MFdiv               1
#define BOARD_PARAM_MFtol               0.05

#ifndef BOARD_PARAM_HFHz
#define BOARD_PARAM_HFHz                16000000
#endif
#ifndef BOARD_PARAM_HFtol
#define BOARD_PARAM_HFtol               0.01
#endif
#ifndef BOARD_PARAM_HFppm
#define BOARD_PARAM_HFppm               20000
#endif

#define BOARD_PARAM_RFHz                32000000
#define BOARD_PARAM_RFdiv               1
#define BOARD_PARAM_RFout               (BOARD_PARAM_RFHz/BOARD_PARAM_RFdiv)
#define BOARD_PARAM_RFtol               0.000002
#define BOARD_PARAM_PLLout              96000000
#define BOARD_PARAM_PLLmult             (BOARD_PARAM_PLLout/BOARD_PARAM_HFHz)
#define BOARD_PARAM_PLLdiv              3
#define BOARD_PARAM_PLLHz               (BOARD_PARAM_PLLout/BOARD_PARAM_PLLdiv)

// Clock divider specs: These are defined at FULL SPEED
#define BOARD_PARAM_AHBCLKDIV           1                       // AHB Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB2CLKDIV          1                       // APB2 Clk = Main CLK / AHBCLKDIV
#define BOARD_PARAM_APB1CLKDIV          1                       // APB1 Clk = Main CLK / AHBCLKDIV

// SWD Interface: Nucleo used SWD in all cases, never JTAG.
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

// B1, B2, B3 are active-low on pins PA0, PA1, PC6
#define BOARD_SW1_PORTNUM               0
#define BOARD_SW1_PORT                  GPIOA
#define BOARD_SW1_PINNUM                0
#define BOARD_SW1_PIN                   (1<<BOARD_SW1_PINNUM)
#define BOARD_SW1_POLARITY              0
#define BOARD_SW1_PULLING               1
#define BOARD_SW2_PORTNUM               0
#define BOARD_SW2_PORT                  GPIOA
#define BOARD_SW2_PINNUM                1
#define BOARD_SW2_PIN                   (1<<BOARD_SW2_PINNUM)
#define BOARD_SW2_POLARITY              0
#define BOARD_SW2_PULLING               1
#define BOARD_SW3_PORTNUM               2
#define BOARD_SW3_PORT                  GPIOC
#define BOARD_SW3_PINNUM                6
#define BOARD_SW3_PIN                   (1<<BOARD_SW3_PINNUM)
#define BOARD_SW3_POLARITY              0
#define BOARD_SW3_PULLING               1

// LED interface is implemented on PB9, PB11, PB15
// Green, Red, and Blue LEDs are available
// These LEDs are also acting as triggers 1, 2, 3
#define BOARD_LEDG_PORTNUM              1                   // PB9
#define BOARD_LEDG_PORT                 GPIOB
#define BOARD_LEDG_PINNUM               9
#define BOARD_LEDG_PIN                  (1<<BOARD_LEDG_PINNUM)
#define BOARD_LEDG_POLARITY             1
#define BOARD_LEDR_PORTNUM              1                   // PB11
#define BOARD_LEDR_PORT                 GPIOB
#define BOARD_LEDR_PINNUM               11
#define BOARD_LEDR_PIN                  (1<<BOARD_LEDR_PINNUM)
#define BOARD_LEDR_POLARITY             1
#define BOARD_LEDB_PORTNUM              1                   // PB15
#define BOARD_LEDB_PORT                 GPIOB
#define BOARD_LEDB_PINNUM               15
#define BOARD_LEDB_PIN                  (1<<BOARD_LEDB_PINNUM)
#define BOARD_LEDB_POLARITY             1

// Supplemental Triggers
// Optional Triggers to use for testing are implemented on:
// PC0, PC1, PC2
#if (BOARD_PARAM_TRIGS > 6)
#   error "This board does not support more than 6 triggers."
#endif
#if (BOARD_PARAM_TRIGS > 3)
#   define BOARD_TRIG5_PORTNUM          2                   // PC0
#   define BOARD_TRIG5_PORT             GPIOC
#   define BOARD_TRIG5_PINNUM           0
#   define BOARD_TRIG5_PIN              (1<<BOARD_TRIG5_PINNUM)
#endif
#if (BOARD_PARAM_TRIGS > 4)
#   define BOARD_TRIG5_PORTNUM          2                   // PC1
#   define BOARD_TRIG5_PORT             GPIOC
#   define BOARD_TRIG5_PINNUM           1
#   define BOARD_TRIG5_PIN              (1<<BOARD_TRIG5_PINNUM)
#endif
#if (BOARD_PARAM_TRIGS > 5)
#   define BOARD_TRIG6_PORTNUM          2                   // PC2
#   define BOARD_TRIG6_PORT             GPIOC
#   define BOARD_TRIG6_PINNUM           2
#   define BOARD_TRIG6_PIN              (1<<BOARD_TRIG6_PINNUM)
#endif


#if BOARD_PARAM(SWEXTIS) > 1
#   error "Only 1 SW-EXTI is defined in the board support header."
#elif BOARD_PARAM(SWEXTIS) > 0
#   define __USE_EXTI15
#   define BOARD_SWEXTI1_NUM            15
#   define BOARD_SWEXTI1_ISR            platform_isr_exti15
#   define BOARD_SWEXTI1_SET()          do { EXTI->SWIER1 = (1<<15); } while (0)
#   define BOARD_SWEXTI1_CLR()          do { EXTI->PR1 = (1<<15); } while (0)
#endif

// RF Front-end GPIO
// STM32WL has internal RF connections for most requirements.
// Front-end control and TCXO control are the main exceptions.
// This is to give power to the TCXO
#define BOARD_RF_TCXOPORTNUM			1		//PB0
#define BOARD_RF_TCXOPORT               GPIOB
#define BOARD_RF_TCXOPINNUM             0
#define BOARD_RF_TCXOPIN  			    (1<<BOARD_RF_TCXOPINNUM)

// Antenna Switch Pins
#define BOARD_RF_CTLPORTNUM             2		//PC4, PC5, PC3
#define BOARD_RF_CTLPORT                GPIOC
#define BOARD_RF_CTL1PORTNUM            BOARD_RF_CTLPORTNUM
#define BOARD_RF_CTL2PORTNUM            BOARD_RF_CTLPORTNUM
#define BOARD_RF_CTL3PORTNUM            BOARD_RF_CTLPORTNUM
#define BOARD_RF_CTL1PORT               BOARD_RF_CTLPORT
#define BOARD_RF_CTL2PORT               BOARD_RF_CTLPORT
#define BOARD_RF_CTL3PORT               BOARD_RF_CTLPORT
#define BOARD_RF_CTL1PINNUM             4
#define BOARD_RF_CTL2PINNUM             5
#define BOARD_RF_CTL3PINNUM             3
#define BOARD_RF_CTL1PIN                (1<<BOARD_RF_CTL1PINNUM)
#define BOARD_RF_CTL2PIN                (1<<BOARD_RF_CTL2PINNUM)
#define BOARD_RF_CTL3PIN                (1<<BOARD_RF_CTL3PINNUM)

// Main serial connections, used at least by MPIPE

// I2C on PA12/PA11 or "D15/D14"
#define BOARD_I2C_PORTNUM               0
#define BOARD_I2C_PORT                  GPIOA
#define BOARD_I2C_ID                    1
#define BOARD_I2C_SCLPINNUM             12
#define BOARD_I2C_SDAPINNUM             11
#define BOARD_I2C_SCLPIN                (1<<BOARD_I2C_SCLPINNUM)
#define BOARD_I2C_SDAPIN                (1<<BOARD_I2C_SDAPINNUM)

// UART on PA2/PA3 to ST-Link (UART2)
#define BOARD_UART_PORTNUM              0
#define BOARD_UART_PORT                 GPIOA
#define BOARD_UART_ID                   2
#define BOARD_UART_TXPINNUM             2
#define BOARD_UART_RXPINNUM             3
#define BOARD_UART_TXPIN                (1<<BOARD_UART_TXPINNUM)
#define BOARD_UART_RXPIN                (1<<BOARD_UART_RXPINNUM)

// EXTUART on PB6/PB7 to D0,D1 (UART1)
// This is for "Extension UART"
#if 0
#   define BOARD_EXTUART_PORTNUM        1
#   define BOARD_EXTUART_PORT           GPIOB
#   define BOARD_EXTUART_ID             1
#   define BOARD_EXTUART_TXPINNUM       6
#   define BOARD_EXTUART_RXPINNUM       7
#   define BOARD_EXTUART_TXPIN          (1<<BOARD_EXTUART_TXPINNUM)
#   define BOARD_EXTUART_RXPIN          (1<<BOARD_EXTUART_RXPINNUM)
#endif



// ADC Analog inputs on Arduino A0-A5
// PB1, PB2, PA10, PB4, PB14, PB13
///@todo the values below are for a different board, do not use
#if 0
#define BOARD_ADC_PINS                  2
#define BOARD_ADC_PORTS                 1
#define BOARD_ADC_0PORTNUM              0
#define BOARD_ADC_0PORT                 GPIOA
#define BOARD_ADC_0CHAN                 0
#define BOARD_ADC_0PINNUM               0
#define BOARD_ADC_0PIN                  (1<<BOARD_ADC_0PINNUM)
#define BOARD_ADC_1PORTNUM              0
#define BOARD_ADC_1PORT                 GPIOA
#define BOARD_ADC_1CHAN                 4
#define BOARD_ADC_1PINNUM               4
#define BOARD_ADC_1PIN                  (1<<BOARD_ADC_1PINNUM)
#endif


// Timer IO: these are multiplexed onto the IOBUS
///@todo the values below are for a different board, do not use
#if 0
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
#endif



/** BOARD Macros (most of them, anyway) <BR>
  * ========================================================================<BR>
  * BOARD Macros are required by the OpenTag STM32L platform implementation.
  * We can assume certain things at the platform level, but not everything,
  * which is why BOARD Macros are required.
  *
  * If your board is implementing a radio (extremely likely), there are likely
  * some additional Board macros down near the radio configuration section.
  *
  * BOARD_PERIPH_INIT()
  * Initialize peripherals that need to be brought-up at startup (before any
  * driver initializations) because they are critical to operation of core
  * OpenTag libraries.  Generally, this is the DMA, Flash controller, and
  * other such tightly-coupled resources.
  */

// CRC
// The built-in CRC engine is used by Opentag for many things, including the
// pseudo-random number generator.  But it is not used in sleep.
#define _CRCCLK_N           RCC_C2AHB1ENR_CRCEN
#define _CRCCLK_LP          0       /* RCC_C2AHB1SMENR_CRCSMEN */

// DMAMUX is important to enable as long as DMA is enabled (always)
#define _DMAMUXCLK_N        RCC_C2AHB1ENR_DMAMUX1EN
#define _DMAMUXCLK_LP       RCC_C2AHB1SMENR_DMAMUX1SMEN

// DMA is used in MEMCPY and thus it could be needed at any time.
// CPU2 only uses DMA2
#define _DMA2CLK_N          RCC_C2AHB1ENR_DMA2EN
#define _DMA2CLK_LP         RCC_C2AHB1SMENR_DMA2SMEN


// FLASH
// Flash is always enabled.  This is also its default set up.
#define _FLASHCLK_N         RCC_C2AHB3ENR_FLASHEN
#define _FLASHCLK_LP        RCC_C2AHB3SMENR_FLASHSMEN

// IPCC
// Enabled by default.  If you are not using CPU1, you can set to 0.
#define _IPCCCLK_N          RCC_C2AHB3ENR_IPCCEN

// HSEM
// Enabled by default.  If you are not using CPU1, you can set to 0.
#define _HSEMCLK_N          RCC_C2AHB3ENR_HSEMEN

// RNG
// The built-in Random number generator engine is used for many things, but
// never in sleep.  The clock is kept on.
#define _RNGCLK_N           RCC_C2AHB3ENR_RNGEN
#define _RNGCLK_LP          0   /* RCC_C2AHB3SMENR_RNGSMEN */

// AES
// AES engine is used only for cryptography (and not random number generation),
// so we leave to the responsibility of the driver to turn it on or off.
#define _AESCLK_N           0   /* RCC_C2AHB3ENR_AESEN */
#define _AESCLK_LP          RCC_C2AHB3SMENR_AESSMEN

// PKA
// Public Key engine is used only for cryptography, so we leave to the
// responsibility of the driver to turn it on or off.
#define _PKACLK_N           0   /* RCC_C2AHB3ENR_PKAEN */
#define _PKACLK_LP          RCC_C2AHB3SMENR_PKASMEN

// All clocks for peripherals on APBs must be managed by their individual
// drivers.


// This board enabled all GPIO on ports A, B, C.
// On startup (_SU), port H is also enabled
// On Stop (LP) Port A should be open for UART
// On Stop-with-RF (LPRF), Ports A, B, C should be open
#define _GPIOCLK_N      (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN)
#define _GPIOCLK_SU     (_GPIOCLK_N | RCC_AHB2ENR_GPIOHEN)
#define _GPIOCLK_LP     (RCC_AHB2SMENR_GPIOASMEN)
#define _GPIOCLK_LPRF   (RCC_AHB2SMENR_GPIOASMEN | RCC_AHB2SMENR_GPIOBSMEN | RCC_AHB2SMENR_GPIOCSMEN)
    

//@note BOARD Macro for Peripheral Clock initialization at startup
static inline void BOARD_PERIPH_INIT(void) {
    // AHB Clock Setup for Active Mode & Sleep
    // Commented-out where the platform defaults (core_main.c) are used.
    RCC->C2AHB1ENR      = (_CRCCLK_N | _DMAMUXCLK_N | _DMA2CLK_N);
    //RCC->C2AHB1SMENR    = (_CRCCLK_LP | 0x07);

    RCC->C2AHB2ENR      = (_GPIOCLK_SU);
    //RCC->C2AHB2SMENR    = (_GPIOCLK_LP);  // Retain sleep defaults at startup

    RCC->C2AHB3ENR      = (_FLASHCLK_N | _IPCCCLK_N | _HSEMCLK_N | _RNGCLK_N);
    RCC->C2AHB3SMENR    = (0x03800000 | _RNGCLK_LP | _AESCLK_LP | _PKACLK_LP);
}



//@note BOARD Macro for DMA peripheral enabling
static inline void BOARD_DMA_CLKON(void) {
#ifdef _DMACLK_DYNAMIC
    RCC->C2AHB1ENR |= RCC_C2AHB1ENR_DMA2EN;
#endif
}

static inline void BOARD_DMA_CLKOFF(void) {
#ifdef _DMACLK_DYNAMIC
    RCC->C2AHB1ENR &= ~RCC_C2AHB1ENR_DMA2EN;
#endif
}



/// BOARD Macro for EXTI initialization.
/// This only considers configurable EXTIs that are specific to the BOARD.
/// - GPIO EXTI Configuration.
/// - Interrupt Pre-masking (almost never used)
static inline void BOARD_EXTI_STARTUP(void) {
    // EXTI0-3: PA0 = SW1, PA1 = SW2, Px2 = Unused, PA3 = UART RX Sense
    SYSCFG->EXTICR[0]   = (0 << 0) \
                        | (0 << 4) \
                        | (0 << 8) \
                        | (0 << 12);
    
    // EXTI4-7: Px4 = Unused, Px5 = Unused, PC6 = SW3, Px7 = Unused
    SYSCFG->EXTICR[1]   = (0 << 0) \
                        | (0 << 4) \
                        | (2 << 8) \
                        | (0 << 12);
                        
    // EXTI8-11:
    SYSCFG->EXTICR[2]   = (0 << 0) \
                        | (0 << 4) \
                        | (0 << 8) \
                        | (0 << 12);

    // EXTI12-15: SWIER is used on EXTI15
    SYSCFG->EXTICR[3]   = (0 << 0) \
                        | (0 << 4) \
                        | (0 << 8) \
                        | (0 << 12);
}



/// BOARD Macro for initializing GPIO ports at startup, according to the
/// connections in the schematic of this board.
///
/// @note many of the pins that are broken-out to the connectors are not
///       assigned in this setup.  They can be assigned as needed.
///
static inline void BOARD_PORT_STARTUP(void) {  
    /// Clock all the GPIOs during setup
    RCC->C2AHB2ENR  = _GPIOCLK_SU;

    
    /// Configure Port A IO.  
    // - A0:  Input (pull-up) for SW1
    // - A1:  Input (pull-up) for SW2
    // - A2:  UART-TX, which is ALT push-pull output
    // - A3:  UART-RX, set to input pull-up.  UART driver manages ALT switching.
    // - A4:  Not configured, set as ANALOG
    // - A5:  Not configured, set as ANALOG
    // - A6:  Not configured, set as ANALOG
    // - A7:  Not configured, set as ANALOG
    // - A8:  Not configured, set as ANALOG
    // - A9:  Not configured, set as ANALOG
    // - A10: Not configured, set as ANALOG
    // - A11: Not configured, set as ANALOG
    // - A12: Not configured, set as ANALOG
    // - A13: SWDIO, using ALT
    // - A14: SWDCLK, using ALT
    // - A15: Available as JTDI, using ALT
    
	GPIOA->BSRR     = BOARD_UART_TXPIN;
	
    GPIOA->MODER    = (GPIO_MODER_INPUT  << (0*2)) \
                    | (GPIO_MODER_INPUT  << (1*2)) \
                    | (GPIO_MODER_ALT    << (2*2)) \
                    | (GPIO_MODER_INPUT  << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_ANALOG << (5*2)) \
                    | (GPIO_MODER_ANALOG << (6*2)) \
                    | (GPIO_MODER_ANALOG << (7*2)) \
                    | (GPIO_MODER_ANALOG << (8*2)) \
                    | (GPIO_MODER_ANALOG << (9*2)) \
                    | (GPIO_MODER_ANALOG << (10*2)) \
                    | (GPIO_MODER_ANALOG << (11*2)) \
                    | (GPIO_MODER_ANALOG << (12*2)) \
                    | (GPIO_MODER_ALT    << (13*2)) \
                    | (GPIO_MODER_ALT    << (14*2)) \
                    | (GPIO_MODER_ALT    << (15*2));
    
    GPIOA->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (2*2)) \
                    | (GPIO_OSPEEDR_10MHz << (3*2)) \
                    | (GPIO_OSPEEDR_40MHz << (13*2)) \
                    | (GPIO_OSPEEDR_40MHz << (14*2));
    
    ///@note sometimes UARTs need to be Open Drain.
    GPIOA->OTYPER   = 0; //(1 << (2)) | (1 << (9));
    
    GPIOA->PUPDR    = (1 << (0*2))
                    | (1 << (1*2))
                    | (1 << (3*2))
                    | (1 << (13*2))
                    | (2 << (14*2));
    
    // USART2 on PA2/PA3 = 7
    GPIOA->AFR[0]   = (7 << (BOARD_UART_TXPINNUM*4)) \
                    | (7 << (BOARD_UART_RXPINNUM*4));

    // JTAG/SWD are default AF (0) on their pins, so this can be commented.
	//GPIOA->AFR[1]   = (0 << ((13-8)*4))
    //                | (0 << ((14-8)*4))
    //                | (0 << ((15-8)*4));

    /// Configure Port B IO.
    // - B0:  TCXO power pin.  Left as floating input.
    // - B1:  Not configured, set as ANALOG
    // - B2:  Not configured, set as ANALOG
    // - B3:  SWO, using ALT
    // - B4:  Not configured, set as ANALOG
    // - B5:  Not configured, set as ANALOG
    // - B6:  Can be EXTUART-TX, but otherwise not configured and set as ANALOG
    // - B7:  Can be EXTUART-RX, but otherwise not configured and set as ANALOG
    // - B8:  Not configured, set as ANALOG
    // - B9:  Green LED2 (Trig 1), set as output
    // - B10: Not configured, set as ANALOG
    // - B11: Red LED3 (Trig 2), set as output
    // - B12: Not configured, set as ANALOG
	// - B13: Not configured, set as ANALOG
    // - B14: Not configured, set as ANALOG
    // - B15: Blue LED1 (Trig 3), set as output

#   if defined(BOARD_EXTUART_PORTNUM)
#       define _MODER_PB6   GPIO_MODER_ALT
#       define _MODER_PB7   GPIO_MODER_ALT
#       define _PUPDR_PB6   1
#       define _OSPEED_PB7  GPIO_OSPEEDR_10MHz
#   else
#       define _MODER_PB6   GPIO_MODER_ANALOG
#       define _MODER_PB7   GPIO_MODER_ANALOG
#       define _PUPDR_PB6   0
#       define _OSPEED_PB7  0
#   endif

    GPIOB->MODER    = (GPIO_MODER_IN        << (0*2)) \
                    | (GPIO_MODER_ANALOG    << (1*2)) \
                    | (GPIO_MODER_ANALOG    << (2*2)) \
                    | (GPIO_MODER_ALT       << (3*2)) \
                    | (GPIO_MODER_ANALOG    << (4*2)) \
                    | (GPIO_MODER_ANALOG    << (5*2)) \
                    | (_MODER_PB6           << (6*2)) \
                    | (_MODER_PB7           << (7*2)) \
                    | (GPIO_MODER_ANALOG    << (8*2)) \
                    | (GPIO_MODER_OUT       << (9*2)) \
                    | (GPIO_MODER_ANALOG    << (10*2)) \
                    | (GPIO_MODER_OUT       << (11*2)) \
                    | (GPIO_MODER_ANALOG    << (12*2)) \
                    | (GPIO_MODER_ANALOG    << (13*2)) \
                    | (GPIO_MODER_ANALOG    << (14*2)) \
                    | (GPIO_MODER_OUT       << (15*2));
    
    // PB0, 1, 2 (RFIOs) should be pullups with high speed, according to STCube
    GPIOB->PUPDR    = (1 << (0*2)) \
                    | (1 << (1*2)) \
                    | (1 << (4*2));
    
    GPIOB->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (0*2)) \
                    | (GPIO_OSPEEDR_10MHz << (1*2)) \
                    | (GPIO_OSPEEDR_40MHz << (3*2)) \
                    | (GPIO_OSPEEDR_10MHz << (4*2)) \
                    | (GPIO_OSPEEDR_10MHz << (8*2));
    
    /// Configure Port C IO.
    /// Port C is used only for USB sense and 32kHz crystal driving
    // - C0:  Trig4, Output
    // - C1:  Trig5, Output
    // - C2:  Trig6, Output
    // - C3:  RFE Ctl 3, Output, Antenna Switch Power
    // - C4:  RFE Ctl 1, Output, see below
    // - C5:  RFE Ctl 2, Output, 00/01/10/11 = Off, HP-TX, RX, LP-TX
    // - C6:  SW3, Pullup Input
    // - C13: Wakeup (unused), pulldown input
    // - C14: LSE pin, setup as output
    // - C15: LSE pin, setup as Alt

    GPIOC->BSRR     = (1 << 4) | 0;

    GPIOC->MODER    = (GPIO_MODER_OUT    << (0*2)) \
                    | (GPIO_MODER_OUT    << (1*2)) \
                    | (GPIO_MODER_OUT    << (2*2)) \
                    | (GPIO_MODER_OUT    << (3*2)) \
                    | (GPIO_MODER_OUT    << (4*2)) \
                    | (GPIO_MODER_OUT    << (5*2)) \
                    | (GPIO_MODER_IN     << (6*2)) \
                    | (GPIO_MODER_IN     << (13*2)) \
                    | (GPIO_MODER_OUT    << (14*2)) \
                    | (GPIO_MODER_ALT    << (15*2));
    
    GPIOC->PUPDR    = (1 << (6*2)) \
                    | (2 << (13*2));
    
    GPIOC->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (3*2)) \
                    | (GPIO_OSPEEDR_10MHz << (4*2)) \
                    | (GPIO_OSPEEDR_10MHz << (5*2));
    
    /// Configure Port H -- There's nothing to do here.


    /// Clock GPIOs for Normal Runtime
    RCC->C2AHB2ENR  = _GPIOCLK_N;
    
#   undef _MODER_PB6
#   undef _MODER_PB7
#   undef _PUPDR_PB6
#   undef _OSPEED_PB7
}


/*
static inline void BOARD_OPEN_FLASH(void* start, void* end) {
/// STM32L0 reserves flash on 4KB boundaries
#   define _F_LAST ((FLASH_AVAILABLE-1) >> 12)
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


static inline void BOARD_RFANT_OFF(void) {
/// PC4:5 set to isolation, then PC3 set to analog (Hi-Z)
    GPIOC->BSRR     = ((1<<4) | (1<<5)) << 16;
    GPIOC->MODER    = GPIOC->MODER | (3 << (3*2));
}


static inline void BOARD_RFANT_ON(void) {
/// PC4:5 set to isolation, then PC3 set to output 1
    GPIOC->BSRR     = (1<<3) | (((1<<4) | (1<<5)) << 16);
    GPIOC->MODER    = (GPIOC->MODER & ~(3 << (3*2))) | (1 << (3*2));
}

static inline void BOARD_RFANT_TX(ot_bool use_boost) {
/// PC4:5 00/01/10/11 = Off, HP-TX, RX, LP-TX
    GPIOC->BSRR = (1<<5) | ((ot_u32)use_boost << 16);
}

static inline void BOARD_RFANT_RX(void) {
    GPIOC->BSRR = (1<<4) | ((1<<5) << 16);
}



static inline void BOARD_RFSPI_CLKON(void) {
    RCC->C2APB3ENR = RCC_APB3ENR_SUBGHZSPIEN;
}

static inline void BOARD_RFSPI_CLKOFF(void) {
    RCC->C2APB3ENR = 0;
}



static inline void BOARD_STOP(ot_int code) {
/// Put the device in STOP2, which is similar to "STOP" on other STM32.
/// Usage:
/// "code" comes from sys_sig_powerdown, but it is usually 0-3.
/// - code = 0 or 1: This function should not be called with 0-1
/// - code = 2: Keep ports alive with peripherals that can be active in STOP
/// - code = 3: Shut down all ports, only
/// Those are the only modes that should call this inline function.
    static const ot_u32 rcc_flags[2] = {
        _GPIOCLK_LPRF,
        _GPIOCLK_LP
    };
    ot_u32  rcc_ahb2enr_saved;
    
    // Shut down unnecessary ports
    rcc_ahb2enr_saved   = RCC->C2AHB2ENR;
    RCC->C2AHB2ENR      = rcc_flags[code&1];
    
    // Disable Systick.  Systick is the devil.
    SysTick->CTRL = 0;
    
    // Prepare SCB->SCR register for DeepSleep instead of regular Sleep.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    
    // Prepare PWR->C2CR1 register for Stop2 Mode
    // - Flash Power Down Enabled
    // - Stop Mode 2
    PWR->C2CR1 = PWR_C2CR1_FPDS | 2;
    
    // Assure PVD is in low power mode.
    ///@note PWR->CR3 is under purview of CPU1, might comment-out later
    PWR->CR3 |= PWR_CR3_ULPEN;

    // Clear any EXTIs -- too late for them now
    ///@todo need to mask erase only the C2 EXTIs
    EXTI->PR1   = 0;
    EXTI->PR2   = 0;
    
    // Enable Interrupts and send "Wait For Interrupt" Cortex-M instruction
    platform_enable_interrupts();
    __WFI();
    
    // ---- Device is in STOP mode here ---- //
    
    // On Wakeup (from STOP) clear flags, re-enable backup register area, 
    // immediately reset SLEEPDEEP bit, and Re-enable Ports
    PWR->C2CR1      = 2;
    SCB->SCR       &= ~((ot_u32)SCB_SCR_SLEEPDEEP_Msk);
    //PWR->CR1     |= PWR_CR1_DBP;      // re-enable backup registers

    ///@note PWR->CR3 is under purview of CPU1, might comment-out later
    PWR->CR3       &= ~PWR_CR3_ULPEN;

    RCC->C2AHB2ENR  = rcc_ahb2enr_saved;
}





static inline void BOARD_PORT_STANDBY() {
///@todo implement this!
}




/// Macro for initializing powering resources, especially the PVD.
static inline void BOARD_POWER_STARTUP(void) {
#   if MCU_PARAM(CPU2PWR)
    PWR->CR1    = (PWR->CR1
                & ~PWR_CR1_LPR
                & ~PWR_CR1_SUBGHZSPINSSSEL
                & ~0x7)
                |  PWR_CR1_FPDS
                |  2;   // Force Stop2 on CPU1

    PWR->CR3    = PWR->CR3
                | PWR_CR3_APC
                | PWR_CR3_RRS;

    PWR->CR4    = PWR->CR4
                & ~PWR_CR4_WRFBUSYP;

    ///@todo Before enabling the SMPS, the SMPS clock detection must be enabled
    ///      in the sub-GHz radio SUBGHZ_SMPSC0R.CLKDE
    PWR->CR5    = 0 /* PWR_CR5_SMPSEN*/
                | 0 /* PWR_CR5_RFEOLEN */ ;

#       if MCU_FEATURE(SVMONITOR)
        ///@todo there's a note in the Ref Manual about SYSCFG->CBR
        //SYSCFG->CBR = ??;

        // - PVM3 checks Vdda against 1.62V.
        // - PLS[2:0] is the voltage threshold, 2.0 - 2.9V (values 0-6).
        //   The setting 5 is 2.8V, which is a good cutoff for Li-Ion.
        PWR->CR2    = 0 /* PWR_CR2_PVME3 */
                    | (5<<1)
                    |  PWR_CR2_PVDE;
#       endif

#   endif

#   if MCU_PARAM(CPU1NULL)
        ///@note EWRFBUSY might not be needed
        PWR->C2CR3  = PWR_C2CR3_EWRFIRQ
                    | PWR_C2CR3_EWRFBUSY
                    | PWR_C2CR3_EWPVD;
#   else
        PWR->C2CR3  = PWR_C2CR3_EIWUL
                    | PWR_C2CR3_EWRFIRQ
                    | PWR_C2CR3_EWRFBUSY;
#   endif

    PWR->SECCFGR    = PWR_SECCFGR_C2EWILA;
}



///@note BOARD Macro for initializing the STM32 Crystal startup routine, done
///      at startup.  It must startup ALL the crystals that you want to run 
///      during normal operation.
static inline void BOARD_XTAL_STARTUP(void) {
}


///@note BOARD Macros for turning-on and turning-off the High-speed XTAL.
///      High speed XTAL on this board is connected to the RF subsystem.
static inline void BOARD_HSXTAL_ON(void) {
}
static inline void BOARD_HSXTAL_OFF(void) {
}




///@todo Create a more intelligent setup that knows how to use the UART
/// BOARD Macros for Com module interrupt vectoring.  Connect these to
/// the Com interface driver you are using.  Check the schematic of your
/// board to see where the Com IRQ lines are routed.
#if (MCU_CONFIG(MPIPEUART) && OT_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE_BREAK))
#   define __USE_EXTI3
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
  * This STM32WL impl is clocked at 48 MHz @ 1.2V core voltage, using the MSI.
  * It also uses the LSE for 32768 Hz generation.
  * The other settings here (HS, PLL, etc) are kept around from other boards,
  * but they are unused.
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

///@note The defaults (commented below) are set in STM32WLxx_config.h  If you
///      want to make changes, you should undef the defines needing changes
///      and define them again below.

///@note STM32WL RTC clocking impl uses a raw period of 1/32768 Hz
///      OpenTag's tick is (generally) 1/1024 sec.
///      Thus the setting here is SHIFT=0, OVERSAMPLE=5
//#define __ISR_RTC_Alarm
//#define OT_GPTIM_ID         'R'
//#define OT_GPTIM            RTC
//#define OT_GPTIM_CLOCK      32768
//#define OT_GPTIM_SHIFT      0
//#define OT_GPTIM_OVERSAMPLE 5
//#define OT_GPTIM_RES        (1024 << OT_GPTIM_SHIFT)    //1024
//#define TI_TO_CLK(VAL)      ((OT_GPTIM_RES/1024)*VAL)
//#define CLK_TO_TI(VAL)      (VAL/(OT_GPTIM_RES/1024))

#if (OT_GPTIM_CLOCK == BOARD_PARAM_LFHz)
#   define OT_GPTIM_ERROR   BOARD_PARAM_LFtol
#else
#   define OT_GPTIM_ERROR   BOARD_PARAM_HFtol
#endif

//this needs to be hard-coded
#define OT_GPTIM_ERRDIV         32768

#define OT_KTIM_IRQ_SRCLINE     BOARD_GPTIM1_PINNUM
#define OT_MACTIM_IRQ_SRCLINE   BOARD_GPTIM2_PINNUM



// Primary LED triggers
#define OT_TRIG1_PORT       BOARD_LEDG_PORT
#define OT_TRIG1_PINNUM     BOARD_LEDG_PINNUM
#define OT_TRIG1_PIN        BOARD_LEDG_PIN
#define OT_TRIG1_POLARITY   BOARD_LEDG_POLARITY
#define OT_TRIG2_PORT       BOARD_LEDR_PORT
#define OT_TRIG2_PINNUM     BOARD_LEDR_PINNUM
#define OT_TRIG2_PIN        BOARD_LEDR_PIN
#define OT_TRIG2_POLARITY   BOARD_LEDR_POLARITY
#define OT_TRIG3_PORT       BOARD_LEDB_PORT
#define OT_TRIG3_PINNUM     BOARD_LEDB_PINNUM
#define OT_TRIG3_PIN        BOARD_LEDB_PIN
#define OT_TRIG3_POLARITY   BOARD_LEDB_POLARITY

// Secondary pin triggers
#if (BOARD_PARAM(TRIGS) >= 4)
#   define OT_TRIG4_PORT       BOARD_TRIG4_PORT
#   define OT_TRIG4_PINNUM     BOARD_TRIG4_PINNUM
#   define OT_TRIG4_PIN        BOARD_TRIG4_PIN
#   define OT_TRIG4_POLARITY   1
#endif
#if (BOARD_PARAM(TRIGS) >= 5)
#   define OT_TRIG5_PORT       BOARD_TRIG5_PORT
#   define OT_TRIG5_PINNUM     BOARD_TRIG5_PINNUM
#   define OT_TRIG5_PIN        BOARD_TRIG5_PIN
#   define OT_TRIG5_POLARITY   1
#endif
#if (BOARD_PARAM(TRIGS) >= 6)
#   define OT_TRIG6_PORT       BOARD_TRIG6_PORT
#   define OT_TRIG6_PINNUM     BOARD_TRIG6_PINNUM
#   define OT_TRIG6_PIN        BOARD_TRIG6_PIN
#   define OT_TRIG6_POLARITY   1
#endif


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

#if (OT_TRIG4_POLARITY != 0)
#   define OT_TRIG4_ON()    OT_TRIG4_PORT->BSRR  = OT_TRIG4_PIN;
#   define OT_TRIG4_OFF()   OT_TRIG4_PORT->BRR   = OT_TRIG4_PIN;
#   define OT_TRIG4_TOG()   OT_TRIG4_PORT->ODR  ^= OT_TRIG4_PIN;
#else 
#   define OT_TRIG4_ON()    OT_TRIG4_PORT->BRR   = OT_TRIG4_PIN;
#   define OT_TRIG4_OFF()   OT_TRIG4_PORT->BSRR  = OT_TRIG4_PIN;
#   define OT_TRIG4_TOG()   OT_TRIG4_PORT->ODR  ^= OT_TRIG4_PIN;
#endif

#if (OT_TRIG5_POLARITY != 0)
#   define OT_TRIG5_ON()    OT_TRIG5_PORT->BSRR  = OT_TRIG5_PIN;
#   define OT_TRIG5_OFF()   OT_TRIG5_PORT->BRR   = OT_TRIG5_PIN;
#   define OT_TRIG5_TOG()   OT_TRIG5_PORT->ODR  ^= OT_TRIG5_PIN;
#else 
#   define OT_TRIG5_ON()    OT_TRIG5_PORT->BRR   = OT_TRIG5_PIN;
#   define OT_TRIG5_OFF()   OT_TRIG5_PORT->BSRR  = OT_TRIG5_PIN;
#   define OT_TRIG5_TOG()   OT_TRIG5_PORT->ODR  ^= OT_TRIG5_PIN;
#endif

#if (OT_TRIG6_POLARITY != 0)
#   define OT_TRIG6_ON()    OT_TRIG6_PORT->BSRR  = OT_TRIG6_PIN;
#   define OT_TRIG6_OFF()   OT_TRIG6_PORT->BRR   = OT_TRIG6_PIN;
#   define OT_TRIG6_TOG()   OT_TRIG6_PORT->ODR  ^= OT_TRIG6_PIN;
#else 
#   define OT_TRIG6_ON()    OT_TRIG6_PORT->BRR   = OT_TRIG6_PIN;
#   define OT_TRIG6_OFF()   OT_TRIG6_PORT->BSRR  = OT_TRIG6_PIN;
#   define OT_TRIG6_TOG()   OT_TRIG6_PORT->ODR  ^= OT_TRIG6_PIN;
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



#ifndef __USE_EXTI0
#   define __USE_EXTI0
#endif
#define OT_SWITCH1_ISR      platform_isr_exti0
#define OT_SWITCH1_PORTNUM  BOARD_SW1_PORTNUM
#define OT_SWITCH1_PORT     BOARD_SW1_PORT
#define OT_SWITCH1_PINNUM   BOARD_SW1_PINNUM
#define OT_SWITCH1_PIN      BOARD_SW1_PIN
#define OT_SWITCH1_POLARITY BOARD_SW1_POLARITY
 
#ifndef __USE_EXTI1
#   define __USE_EXTI1
#endif
#define OT_SWITCH2_ISR      platform_isr_exti1
#define OT_SWITCH2_PORTNUM  BOARD_SW2_PORTNUM
#define OT_SWITCH2_PORT     BOARD_SW2_PORT
#define OT_SWITCH2_PINNUM   BOARD_SW2_PINNUM
#define OT_SWITCH2_PIN      BOARD_SW2_PIN
#define OT_SWITCH2_POLARITY BOARD_SW2_POLARITY

#ifndef __USE_EXTI6
#   define __USE_EXTI6
#endif
#define OT_SWITCH3_ISR      platform_isr_exti6
#define OT_SWITCH3_PORTNUM  BOARD_SW3_PORTNUM
#define OT_SWITCH3_PORT     BOARD_SW3_PORT
#define OT_SWITCH3_PINNUM   BOARD_SW3_PINNUM
#define OT_SWITCH3_PIN      BOARD_SW3_PIN
#define OT_SWITCH3_POLARITY BOARD_SW3_POLARITY



/** STM32L0 MPipe Setup <BR>
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

#   ifdef __USE_DMA2_CHAN7
#       error "Resource collision on DMA2-Channel7 (MPIPE)."
#   else
#       define MPIPE_DMANUM         2
#       define MPIPE_DMA            DMA2
#       define MPIPE_DMA_RXCHAN_ID  14
#       define MPIPE_DMA_TXCHAN_ID  14
#       define __USE_DMA2_CHAN7
#   endif

#	if MCU_CONFIG(MPIPEUART)
#       define MPIPE_UART_ID        BOARD_UART_ID
#       define MPIPE_UART_PORTNUM   BOARD_UART_PORTNUM
#       define MPIPE_UART_PORT      BOARD_UART_PORT
#       define MPIPE_UART_RXPIN     BOARD_UART_RXPIN
#       define MPIPE_UART_TXPIN     BOARD_UART_TXPIN
#       define MPIPE_RTS_PORT       BOARD_UART_PORT
#       define MPIPE_CTS_PORT       BOARD_UART_PORT
#       define MPIPE_RTS_PIN        BOARD_UART_RTSPIN
#       define MPIPE_CTS_PIN        BOARD_UART_CTSPIN
#       define MPIPE_UART_PINS      (MPIPE_UART_RXPIN | MPIPE_UART_TXPIN)
#       if (MPIPE_UART_ID != 2)
#           error "MPIPE UART must be on USART2 for this board."
#       endif
#       ifndef __ISR_USART2
#           define __ISR_USART2
#       endif
#       define MPIPE_UART           USART2

#	elif MCU_CONFIG(MPIPEI2C)
#       define MPIPE_I2C_ID         BOARD_I2C_ID
#       define MPIPE_I2C            I2C2
#       define MPIPE_I2C_PORTNUM    BOARD_I2C_PORTNUM
#       define MPIPE_I2C_PORT       BOARD_I2C_PORT
#       define MPIPE_I2C_RXPIN      BOARD_I2C_SCLPIN
#       define MPIPE_I2C_TXPIN      BOARD_I2C_SDAPIN
#       define MPIPE_I2C_PINS       (MPIPE_I2C_RXPIN | MPIPE_I2C_TXPIN)
#       if (MPIPE_I2C_ID != 2)
#           error "MPIPE I2C must be on I2C2 for this board."
#       endif
#   endif

#endif








/** RF interface setup <BR>
  * ========================================================================<BR>
  * Unlike with external radios, the signals in the STM32WL between radio core
  * and MCU are internal, not GPIOs.  So there is no GPIO configuration to do,
  * all configurations can happen uniquely inside driver code.
  *
  * - STM32WL has integrated RF accessible via AHB3.
  * - The SUBGHZSPI driver implementation is:
  *   - blocking (no interrupts)
  *   - uses DMAs
  * - The Available IRQs are internal signals rather than GPIOs
  *   - Set PWR->C2CR3.EWRFIRQ to enable the Radio DIO IRQ
  *   - Set PWR->C2CR3.EWRFBUSY to enable the Radio Busy to make IRQ
  *   - EXTI line 44 is the Global RFIRQ line (DIO0, 1, 2)
  *   - EXTI line 45 is the RFBUSY line
  * - The underlying signals are available:
  *   - RF-Reset (set) via RCC->CSR.RFRST
  *   - RF-Reset (check) via RCC->CSR.RFRSTF
  *   - Individual DIO signals NOT available, status must be read via SPI
  *   - Check PWR->SR2.RFBUSYS for radio state machine status (see 126x datasheet)
  *   - Check PWR->SR2.RFBUSYMS as RFBUSYS + RFSPI bus busy
  */
 
#ifdef __USE_RADIO
// RADIO_SPI must be SPI3 (SUBGHZSPI) on this board
#   define RADIO_SPI_ID     3
#   define RADIO_SPI        SUBGHZSPI

#   if (defined(__USE_DMA2_CHAN5) || defined(__USE_DMA2_CHAN6))
#       error "RADIO SPI implementation requires DMA2-Ch5, DMA2-Ch6 to be free"

#   else
#       define __USE_DMA2_CHAN5
#       define __USE_DMA2_CHAN6
#       define __N_ISR_DMA2_Channel5
#       define __N_ISR_DMA2_Channel6
#   endif


#endif



/** Boilerplate STM32L DMA Memcpy Setup <BR>
  * ========================================================================<BR>
  * Memcpy is a MEM2MEM function of the DMA, so it can take any channel.
  *
  */
#if (MCU_CONFIG_MEMCPYDMA != ENABLED)
#   warning "Not using DMA for MEMCPY, a sub-optimal design choice."
#elif !defined(__USE_DMA2_CHAN7)
#   define MEMCPY_DMA_CHAN_ID   14
#   define MEMCPY_DMA           DMA2
#   define MEMCPY_DMACHAN       DMA2_Channel7
#   define __USE_DMA2_CHAN7
#elif !defined(__USE_DMA2_CHAN6)
#   define MEMCPY_DMA_CHAN_ID   13
#   define MEMCPY_DMA           DMA2
#   define MEMCPY_DMACHAN       DMA2_Channel6
#   define __USE_DMA2_CHAN6
#elif !defined(__USE_DMA2_CHAN5)
#   define MEMCPY_DMA_CHAN_ID   12
#   define MEMCPY_DMA           DMA2
#   define MEMCPY_DMACHAN       DMA2_Channel5
#   define __USE_DMA2_CHAN5
#elif !defined(__USE_DMA2_CHAN4)
#   define MEMCPY_DMA_CHAN_ID   11
#   define MEMCPY_DMA           DMA2
#   define MEMCPY_DMACHAN       DMA2_Channel4
#   define __USE_DMA2_CHAN4
#elif !defined(__USE_DMA2_CHAN3)
#   define MEMCPY_DMA_CHAN_ID   10
#   define MEMCPY_DMA           DMA2
#   define MEMCPY_DMACHAN       DMA2_Channel3
#   define __USE_DMA2_CHAN3
#elif !defined(__USE_DMA2_CHAN2)
#   define MEMCPY_DMA_CHAN_ID   9
#   define MEMCPY_DMA           DMA2
#   define MEMCPY_DMACHAN       DMA2_Channel2
#   define __USE_DMA2_CHAN2
#elif !defined(__USE_DMA2_CHAN1)
#   define MEMCPY_DMA_CHAN_ID   8
#   define MEMCPY_DMA           DMA2
#   define MEMCPY_DMACHAN       DMA2_Channel1
#   define __USE_DMA2_CHAN1
#else
#   error "There is no available DMA Channel to allocate to MEMCPY."
#endif






/** EXTI Line Utilization declaration <BR>
  * ========================================================================<BR>
  * For this board, we do not open-up the rest of the EXTIs for application
  * usage, because the purpose of OpenTag on the STM32WL M0+ core is only to
  * provide a radio layer.  User apps should be in the M4.

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
*/

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
#	include <io/stm32wl_lora/interface.h>
#endif


#endif
