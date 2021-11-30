/* Copyright 2013-2014 JP Norair
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
  */
/**
  * @file       /board/stm32l0xx/discovery_LoRa.h
  * @author     JP Norair
  * @version    R100
  * @date       10 Apr 2017
  * @brief      Board Configuration for ST Discovery LoRa 
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
  

#ifndef __discovery_LoRa_H
#define __discovery_LoRa_H

/// MCU definition for the board must be the first thing
#define __STM32L072CZ__

#include <app/app_config.h>
#include <platform/hw/STM32L0xx_config.h>
#include <platform/interrupts.h>

#if OT_FEATURE(M2)
#   define __USE_RADIO
#endif

/// You can add this shield
//#define __EXT_GNSS__


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
#		define __SX127x__
#		define __SX1276__
#       define __SX127x_TXSW__
#       define __SX127x_RXSW__
#       define __SX127x_PABOOST__
#		define __SX127x_20dBm__
#       include <io/sx127x/config.h>
#   endif
#endif

#if defined(__EXT_GNSS__)
#   if defined(__UBX_GNSS__)
#       define __UBX_GNSS__
#       define __UBX_M8__
#       define __UBX_MAXM8x__
#       define __UBX_MAXM8C__
#       include <io/ubx_gnss.h>
#   else // __NULL_GNSS__
#		define __NULL_GNSS__
#       include <io/gnss_null/config.h>
#	endif
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
#define BOARD_FEATURE_MPIPE_BREAK       DISABLED                    // Send/receive leading break for wakeup
#define BOARD_FEATURE_MPIPE_DIRECT      (BOARD_FEATURE_MPIPE_BREAK != ENABLED) 

#define BOARD_FEATURE_I2C               DISABLED

// If you have installed GNSS extension board from Haystack
#if defined(__UBX_GNSS__)
#	define BOARD_FEATURE_UBX_GNSS       ENABLED       // Ublox GNSS
#	define BOARD_FEATURE_GNSS_SDN       DISABLED      // GNSS can be completely powered-off
#elif defined(__NULL_GNSS__)
#	define BOARD_FEATURE_GNSSNULL 		ENABLED
#endif

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
#ifndef BOARD_FEATURE_HFCRS
#define BOARD_FEATURE_HFCRS             (BOARD_FEATURE_HFXTAL != ENABLED)       // Use STM32L0's Clock-Recovery-System for USB
#endif
#ifndef BOARD_FEATURE_RFXTAL
#define BOARD_FEATURE_RFXTAL            ENABLED                                 // XTAL for RF chipset
#endif
#ifndef BOARD_FEATURE_RFXTALOUT
#define BOARD_FEATURE_RFXTALOUT         DISABLED
#endif
#ifndef BOARD_FEATURE_PLL
#define BOARD_FEATURE_PLL               (MCU_CONFIG_MULTISPEED == ENABLED)
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
#define BOARD_FEATURE_FLANKSPEED        (MCU_CONFIG_MULTISPEED == ENABLED)
#endif
#ifndef BOARD_FEATURE_FLANKXTAL
#define BOARD_FEATURE_FLANKXTAL         (BOARD_FEATURE_HFCRS != ENABLED)
#endif
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
#if !defined(BOARD_FEATURE_RF_PABOOST)
#	if !defined(__SX127x_PABOOST__)
#	define BOARD_FEATURE_RF_PABOOST		DISABLED
#	else
#	define BOARD_FEATURE_RF_PABOOST		ENABLED
#	endif
#endif

// Number of Triggers/LEDs
#if (BOARD_FEATURE_I2C)
#   define BOARD_PARAM_TRIGS            4
#else
#   define BOARD_PARAM_TRIGS            6
#endif

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

#ifndef BOARD_PARAM_HFHz
#define BOARD_PARAM_HFHz                16000000
#endif
#ifndef BOARD_PARAM_HFtol
#define BOARD_PARAM_HFtol               0.02
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

// SW1 is implemented on PB2, active low.
#define BOARD_SW1_PORTNUM               1                   // Port B
#define BOARD_SW1_PORT                  GPIOB
#define BOARD_SW1_PINNUM                2
#define BOARD_SW1_PIN                   (1<<BOARD_SW1_PINNUM)
#define BOARD_SW1_POLARITY              0
#define BOARD_SW1_PULLING               0

// TEST0, TEST1 pins are on PB14, PB15.
// They are not necessary, but Haystack uses them for manufacturing tests.
// They are usually outputs but they could be pull-up inputs also.
#define BOARD_TEST0_PORTNUM             1                   // Port B
#define BOARD_TEST0_PORT                GPIOB
#define BOARD_TEST0_PINNUM              14
#define BOARD_TEST0_PIN                 (1<<BOARD_TEST0_PINNUM)
#define BOARD_TEST0_POLARITY            0
#define BOARD_TEST0_PULLING             1
#define BOARD_TEST1_PORTNUM             1                   // Port B
#define BOARD_TEST1_PORT                GPIOB
#define BOARD_TEST1_PINNUM              15
#define BOARD_TEST1_PIN                 (1<<BOARD_TEST1_PINNUM)
#define BOARD_TEST1_POLARITY            0
#define BOARD_TEST1_PULLING             1

// LED interface is implemented on PB5 and PB7.
// Additionally, there is an optional Red-LED spec'ed on PA5
// The Blue LED on PB6 is available, but we use it with our GNSS extension board
// to show TIMEPULSE output from GNSS.
#define BOARD_LEDG_PORTNUM              1                   // PB5
#define BOARD_LEDG_PORT                 GPIOB
#define BOARD_LEDG_PINNUM               5
#define BOARD_LEDG_PIN                  (1<<BOARD_LEDG_PINNUM)
#define BOARD_LEDG_POLARITY             1
#define BOARD_LEDO_PORTNUM              1                   // PB7
#define BOARD_LEDO_PORT                 GPIOB
#define BOARD_LEDO_PINNUM               7
#define BOARD_LEDO_PIN                  (1<<BOARD_LEDO_PINNUM)
#define BOARD_LEDO_POLARITY             1
#define BOARD_LEDR_PORTNUM              0                   // PA5
#define BOARD_LEDR_PORT                 GPIOA
#define BOARD_LEDR_PINNUM               5
#define BOARD_LEDR_PIN                  (1<<BOARD_LEDR_PINNUM)
#define BOARD_LEDR_POLARITY             1
#define BOARD_LEDB_PORTNUM              1                   // PB6
#define BOARD_LEDB_PORT                 GPIOB
#define BOARD_LEDB_PINNUM               6
#define BOARD_LEDB_PIN                  (1<<BOARD_LEDB_PINNUM)
#define BOARD_LEDB_POLARITY             1

// Supplemental Triggers
// These are optional and may be used instead of I2C if the application calls for more
// triggers, and obviously doesn't call for I2C
#if (BOARD_PARAM_TRIGS > 4)
#   define BOARD_TRIG5_PORTNUM          1                   // PB8
#   define BOARD_TRIG5_PORT             GPIOB
#   define BOARD_TRIG5_PINNUM           8
#   define BOARD_TRIG5_PIN              (1<<BOARD_TRIG5_PINNUM)
#endif
#if (BOARD_PARAM_TRIGS > 5)
#   define BOARD_TRIG6_PORTNUM          1                   // PB9
#   define BOARD_TRIG6_PORT             GPIOB
#   define BOARD_TRIG6_PINNUM           9
#   define BOARD_TRIG6_PIN              (1<<BOARD_TRIG6_PINNUM)
#endif

// LoRa Module SX127x Bus
#ifdef __USE_RADIO
#   define BOARD_RFGPIO_0PORTNUM        1        // PB4
#   define BOARD_RFGPIO_0PORT           GPIOB
#   define BOARD_RFGPIO_0PINNUM         4
#   define BOARD_RFGPIO_0PIN            (1<<BOARD_RFGPIO_0PINNUM)
#   define BOARD_RFGPIO_1PORTNUM        1        // PB1
#   define BOARD_RFGPIO_1PORT           GPIOB
#   define BOARD_RFGPIO_1PINNUM         1
#   define BOARD_RFGPIO_1PIN            (1<<BOARD_RFGPIO_1PINNUM)
#   define BOARD_RFGPIO_2PORTNUM        1        // PB0
#   define BOARD_RFGPIO_2PORT           GPIOB
#   define BOARD_RFGPIO_2PINNUM         0
#   define BOARD_RFGPIO_2PIN            (1<<BOARD_RFGPIO_2PINNUM)
#   define BOARD_RFGPIO_3PORTNUM        2        // PC13
#   define BOARD_RFGPIO_3PORT           GPIOC
#   define BOARD_RFGPIO_3PINNUM         13
#   define BOARD_RFGPIO_3PIN            (1<<BOARD_RFGPIO_3PINNUM)

// DIO4 and DIO5 aren't generally connected
#   define BOARD_RFGPIO_4PORTNUM        0        // PA5
#   define BOARD_RFGPIO_4PORT           GPIOA
#   define BOARD_RFGPIO_4PINNUM         5
#   define BOARD_RFGPIO_4PIN            (1<<BOARD_RFGPIO_4PINNUM)
#   define BOARD_RFGPIO_5PORTNUM        0        // PA4
#   define BOARD_RFGPIO_5PORT           GPIOA
#   define BOARD_RFGPIO_5PINNUM         4
#   define BOARD_RFGPIO_5PIN            (1<<BOARD_RFGPIO_5PINNUM)

// This is to give power to the TCXO
#	define BOARD_RFTCXO_PORTNUM			0		//PA12
#	define BOARD_RFTCXO_PORT            GPIOA
#	define BOARD_RFTCXO_PINNUM          12
#	define BOARD_RFTCXO_PIN  			(1<<BOARD_RFTCXO_PINNUM)

// Antenna Switch Pins
#	define BOARD_RFANT_RXPORTNUM        0		//PA1
#	define BOARD_RFANT_RXPORT           GPIOA
#	define BOARD_RFANT_RXPINNUM         1
#	define BOARD_RFANT_RXPIN            (1<<BOARD_RFANT_RXPINNUM)
#	define BOARD_RFBOOST_PORTNUM      	2		//PC1
#	define BOARD_RFBOOST_PORT         	GPIOC
#	define BOARD_RFBOOST_PINNUM       	1
#	define BOARD_RFBOOST_PIN          	(1<<BOARD_RFBOOST_PINNUM)
#	define BOARD_RFRFO_PORTNUM          2		//PC2
#	define BOARD_RFRFO_PORT             GPIOC
#	define BOARD_RFRFO_PINNUM           2
#	define BOARD_RFRFO_PIN              (1<<BOARD_RFRFO_PINNUM)

#   define BOARD_RFCTL_RESETPORTNUM     2       //PC0
#   define BOARD_RFCTL_RESETPORT        GPIOC
#   define BOARD_RFCTL_RESETPINNUM      0       
#   define BOARD_RFCTL_RESETPIN         (1<<BOARD_RFCTL_RESETPINNUM)
#   define BOARD_RFSPI_NSSPORTNUM       0       //PA15
#   define BOARD_RFSPI_NSSPORT          GPIOA
#   define BOARD_RFSPI_NSSPINNUM        15       
#   define BOARD_RFSPI_NSSPIN           (1<<BOARD_RFSPI_NSSPINNUM)
#   define BOARD_RFSPI_ID               1       //SPI1
#   define BOARD_RFSPI_PORTNUM          0       //Port A
#   define BOARD_RFSPI_PORT             GPIOA
#   define BOARD_RFSPI_MOSIPINNUM       7       // "D11" : PA7
#   define BOARD_RFSPI_MISOPINNUM       6       // "D12" : PA6
#   define BOARD_RFSPI_SCLKPINNUM       3       // "D13" : PB3
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

// UART on PA2/PA3 or "D1/D0" (UART2)
#define BOARD_UART_PORTNUM              0
#define BOARD_UART_PORT                 GPIOA
#define BOARD_UART_ID                   2
#define BOARD_UART_TXPINNUM             2
#define BOARD_UART_RXPINNUM             3
#define BOARD_UART_TXPIN                (1<<BOARD_UART_TXPINNUM)
#define BOARD_UART_RXPIN                (1<<BOARD_UART_RXPINNUM)

// EXTUART on PA9/PA10 (UART1)
// This is for "Extension UART"
#define BOARD_EXTUART_PORTNUM           0
#define BOARD_EXTUART_PORT              GPIOA
#define BOARD_EXTUART_ID                1
#define BOARD_EXTUART_TXPINNUM          9
#define BOARD_EXTUART_RXPINNUM          10
#define BOARD_EXTUART_TXPIN             (1<<BOARD_EXTUART_TXPINNUM)
#define BOARD_EXTUART_RXPIN             (1<<BOARD_EXTUART_RXPINNUM)

#if defined(__EXT_GNSS__)
#   define BOARD_GNSS_RESETPORTNUM      0       //PA8
#   define BOARD_GNSS_RESETPORT         GPIOA
#   define BOARD_GNSS_RESETPINNUM       8       
#   define BOARD_GNSS_RESETPIN          (1<<BOARD_GNSS_RESETPINNUM)

#   define BOARD_GNSS_INT0PORTNUM       1       //PB13
#   define BOARD_GNSS_INT0PORT          GPIOB
#   define BOARD_GNSS_INT0PINNUM        13       
#   define BOARD_GNSS_INT0PIN           (1<<BOARD_GNSS_INT0PINNUM)

#   define BOARD_GNSS_PULSEPORTNUM      1       //PB6
#   define BOARD_GNSS_PULSEPORT         GPIOB
#   define BOARD_GNSS_PULSEPINNUM       6       
#   define BOARD_GNSS_PULSEPIN          (1<<BOARD_GNSS_PULSEPINNUM)

    static inline void BOARD_GNSS_TURNOFF(void) {
    }
    static inline void BOARD_GNSS_TURNON(void) {
    }
    static inline void BOARD_GNSS_INTON(void) {
        BOARD_GNSS_INT0PORT->BSRR = BOARD_GNSS_INT0PIN;
    }
    static inline void BOARD_GNSS_INTOFF(void) {
        BOARD_GNSS_INT0PORT->BRR = BOARD_GNSS_INT0PIN;
    }

#else
#   define BOARD_GNSS_RESETPIN          0

#endif


// USB is always on PA11/A12
#define BOARD_USB_PORTNUM               0
#define BOARD_USB_PORT                  GPIOA
#define BOARD_USB_DMPINNUM              11
#define BOARD_USB_DPPINNUM              12
#define BOARD_USB_DMPIN                 (1<<BOARD_USB_DMPINNUM)
#define BOARD_USB_DPPIN                 (1<<BOARD_USB_DPPINNUM)

// ADC Analog inputs on PA0, PA4 (A0, A2 in 'duino lingo)
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

// This board enabled all GPIO on ports A, B, C.
// On startup (_SU), port H is also enabled
// On Stop (LP) Port A should be open for UART or USB
// On Stop-with-RF (LPRF), Ports A, B, C should be open
#define _GPIOCLK_N      (RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN)
#define _GPIOCLK_SU     (_GPIOCLK_N | RCC_IOPENR_GPIOHEN)
#define _GPIOCLK_LP     (RCC_IOPENR_GPIOAEN)
#define _GPIOCLK_LPRF   (RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN)

#define _IOPENR_STARTUP  (_DMACLK_N | _MIFCLK_N | _CRCCLK_N | _GPIOCLK_SU)
#define _IOPENR_RUNTIME  (_CRYPCLK_N | _MIFCLK_N | _CRCCLK_N | _GPIOCLK_N)

#ifndef _APB2_PERIPH_CLK
#   define _APB2_PERIPH_CLK     0
#endif
    

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
    RCC->APB2ENR   = (RCC_APB2ENR_DBGMCUEN | RCC_APB2ENR_SYSCFGEN | _APB2_PERIPH_CLK);
#   else
    RCC->APB2ENR   = (RCC_APB2ENR_SYSCFGEN | _APB2_PERIPH_CLK);
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
///      - Radio EXTIs: {GPIO-0, -1, -3} : {PB4, PB1, PC13}
static inline void BOARD_EXTI_STARTUP(void) {
    // EXTI0-3: A0, RF1 (B1), B2, A3 (UART RX Trigger)
    SYSCFG->EXTICR[0]   = (0 << 0) \
                        | (BOARD_RFGPIO_1PORTNUM << 4) \
                        | (1 << 8) \
                        | (0 << 12);
    
    // EXTI4-7: RF0 (B4), B5, B6, C7
    SYSCFG->EXTICR[1]   = (BOARD_RFGPIO_0PORTNUM << 0) \
                        | (1 << 4) \
                        | (1 << 8) \
                        | (2 << 12);
                        
    // EXTI8-11: A8, B9, A10 (UART RX Trigger), A11
    SYSCFG->EXTICR[2]   = (0 << 0) \
                        | (1 << 4) \
                        | (0 << 8) \
                        | (0 << 12);

    // EXTI12-15: A12, RF3 (C13), B14, A15
    SYSCFG->EXTICR[3]   = (0 << 0) \
                        | (BOARD_RFGPIO_3PORTNUM << 4) \
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
    // - A0:  Analog Input
    // - A1:  RF RX Switch, output
    // - A2:  UART-TX, which is ALT push-pull output
    // - A3:  UART-RX, which is ALT pull up input, 
    // - A4:  Analog Input for Battery sensing
    // - A5:  Red LED
    // - A6:7 Radio MISO/MOSI
    // - A8:  GNSS RESET Pin -- Output Push-Pull
    // - A9:  EXT-UART-TX (managed by driver)
    // - A10: EXT-UART-RX (managed by driver)
    // - A11: Would be USB-DM, but not used here
    // - A12: TCXO enable (push pull output, default high)
    // - A13:14 are SWD, set to ALT
    // - A15: Radio SPI CS.  Set to Output, default high
    
    // Set this for startup, the driver can deal with it after startup
	GPIOA->BSRR     = (BOARD_RFTCXO_PIN \
	                |  BOARD_EXTUART_TXPIN \
	                |  BOARD_GNSS_RESETPIN \
	                |  BOARD_RFSPI_NSSPIN);
	
    GPIOA->MODER    = (GPIO_MODER_ANALOG << (0*2)) \
                    | (GPIO_MODER_OUT    << (1*2)) \
                    | (GPIO_MODER_ALT    << (2*2)) \
                    | (GPIO_MODER_ALT    << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_OUT    << (5*2)) \
                    | (GPIO_MODER_ALT    << (6*2)) \
                    | (GPIO_MODER_ALT    << (7*2)) \
                    | (GPIO_MODER_OUT    << (8*2)) \
                    | (GPIO_MODER_OUT    << (9*2)) \
                    | (GPIO_MODER_IN     << (10*2)) \
                    | (GPIO_MODER_ANALOG << (11*2)) \
                    | (GPIO_MODER_OUT    << (12*2)) \
                    | (GPIO_MODER_ALT    << (13*2)) \
                    | (GPIO_MODER_ALT    << (14*2)) \
                    | (GPIO_MODER_OUT    << (15*2));
    
    GPIOA->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (1*2)) \
                    | (GPIO_OSPEEDR_10MHz << (2*2)) \
                    | (GPIO_OSPEEDR_10MHz << (3*2)) \
                    | (GPIO_OSPEEDR_10MHz << (5*2)) \
                    | (GPIO_OSPEEDR_40MHz << (6*2)) \
                    | (GPIO_OSPEEDR_40MHz << (7*2)) \
                    | (GPIO_OSPEEDR_10MHz << (9*2)) \
                    | (GPIO_OSPEEDR_10MHz << (10*2)) \
                    | (GPIO_OSPEEDR_40MHz << (13*2)) \
                    | (GPIO_OSPEEDR_40MHz << (14*2));
    
//    GPIOA->PUPDR    = (1 << (3*2)) \
//                    | (1 << (10*2)) \
//                    | (1 << (13*2)) \
//                    | (2 << (14*2));
    
    ///@note sometimes extended UART needs to have open-drain transmitter
    GPIOA->OTYPER   = 0; //(1 << (9));
    
    GPIOA->PUPDR    = (1 << (3*2)) \
                    | (1 << (9*2)) \
    				| (1 << (10*2)) \
                    | (1 << (13*2)) \
                    | (2 << (14*2));
    
    GPIOA->AFR[0]   = (4 << (BOARD_UART_TXPINNUM*4)) \
                    | (4 << (BOARD_UART_RXPINNUM*4)) \
                    | (0 << ((BOARD_RFSPI_MOSIPINNUM)*4)) \
                    | (0 << ((BOARD_RFSPI_MISOPINNUM)*4));

    /// If USB is used, set ALT.
    ///@todo validate this ALT implementation.
#   if (MCU_CONFIG(USB))
    GPIOA->AFR[1]   = (4 << ((BOARD_EXTUART_TXPINNUM-8)*4)) \
                    | (4 << ((BOARD_EXTUART_RXPINNUM-8)*4)) \
    				| (10 << ((BOARD_USB_DMPINNUM-8)*4)) \
                    | (10 << ((BOARD_USB_DPPINNUM-8)*4));
#   else
	GPIOA->AFR[1]   = (4 << ((BOARD_EXTUART_TXPINNUM-8)*4)) \
                    | (4 << ((BOARD_EXTUART_RXPINNUM-8)*4));
#	endif

    /// Configure Port B IO.
    // - B0:  RF GPIO 2 -- Input
    // - B1:  RF GPIO 1 -- Input
    // - B2:  User Switch -- Input Hi-Z
    // - B3:  RF SCLK (ALT)
    // - B4:  RF GPIO 0 -- Input
    // - B5:  Green LED
    // - B6:  Blue LED: Use as Hi-Z Input with GNSS attached
    // - B7:  Orange LED
    // - B8:9 I2C Bus (Open Drain) or Triggers 5 & 6
    // - B10:12 Unused, set to Analog
	// - B13: GNSS INT pin, an output
    // - B14:15 are Test outputs
#   if defined(__EXT_GNSS__)
#   define MODER_PB6    GPIO_MODER_IN
#   else
#   define MODER_PB6    GPIO_MODER_OUT
#   endif
#   if (BOARD_FEATURE_I2C) 
    GPIOB->OTYPER   = (1 << (8)) | (1 << (9));
    GPIOB->BSRR     = (1 << (8)) | (1 << (9));
#   endif
    GPIOB->MODER    = (GPIO_MODER_IN     << (0*2)) \
                    | (GPIO_MODER_IN     << (1*2)) \
                    | (GPIO_MODER_IN     << (2*2)) \
                    | (GPIO_MODER_ALT    << (3*2)) \
                    | (GPIO_MODER_IN     << (4*2)) \
                    | (GPIO_MODER_OUT    << (5*2)) \
                    | (MODER_PB6         << (6*2)) \
                    | (GPIO_MODER_OUT    << (7*2)) \
                    | (GPIO_MODER_OUT    << (8*2)) \
                    | (GPIO_MODER_OUT    << (9*2)) \
                    | (GPIO_MODER_ANALOG << (10*2)) \
                    | (GPIO_MODER_ANALOG << (11*2)) \
                    | (GPIO_MODER_ANALOG << (12*2)) \
                    | (GPIO_MODER_OUT    << (13*2)) \
                    | (GPIO_MODER_OUT    << (14*2)) \
                    | (GPIO_MODER_OUT    << (15*2));
    
    // PB0, 1, 2 (RFIOs) should be pullups with high speed, according to STCube
    GPIOB->PUPDR    = (1 << (0*2)) \
                    | (1 << (1*2)) \
                    | (1 << (4*2));
    
    GPIOB->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (0*2)) \
                    | (GPIO_OSPEEDR_10MHz << (1*2)) \
                    | (GPIO_OSPEEDR_40MHz << (3*2)) \
                    | (GPIO_OSPEEDR_10MHz << (4*2)) \
                    | (GPIO_OSPEEDR_10MHz << (8*2)) \
                    | (GPIO_OSPEEDR_10MHz << (9*2));
                    
	GPIOB->AFR[0]   = (0 << ((BOARD_RFSPI_SCLKPINNUM)*4));
    
    /// Configure Port C IO.
    /// Port C is used only for USB sense and 32kHz crystal driving
    // - C0:    Radio Reset, which starts as a floating input and the driver later sorts-out
    // - C1:    RF Boost setting, output
    // - C2:    RF RFO (tx switch), output
    // - C3:12  Unused -- Analog
    // - C13    RF GPIO3 Input
    // - C14:15 are 32kHz crystal driving, they are set in a particular way

    GPIOC->MODER    = (GPIO_MODER_IN     << (0*2)) \
                    | (GPIO_MODER_OUT    << (1*2)) \
                    | (GPIO_MODER_OUT    << (2*2)) \
                    | (GPIO_MODER_ANALOG << (3*2)) \
                    | (GPIO_MODER_ANALOG << (4*2)) \
                    | (GPIO_MODER_ANALOG << (5*2)) \
                    | (GPIO_MODER_ANALOG << (6*2)) \
                    | (GPIO_MODER_ANALOG << (7*2)) \
                    | (GPIO_MODER_ANALOG << (8*2)) \
                    | (GPIO_MODER_ANALOG << (9*2)) \
                    | (GPIO_MODER_ANALOG << (10*2)) \
                    | (GPIO_MODER_ANALOG << (11*2)) \
                    | (GPIO_MODER_ANALOG << (12*2)) \
                    | (GPIO_MODER_IN     << (13*2)) \
                    | (GPIO_MODER_OUT    << (14*2)) \
                    | (GPIO_MODER_ALT    << (15*2));
    
    // PC13 (RFIO3) should be pullup and high speed, according to ST
    GPIOC->PUPDR    = (1 << (13*2));
    GPIOC->OSPEEDR  = (GPIO_OSPEEDR_10MHz << (1*2)) \
                    | (GPIO_OSPEEDR_10MHz << (2*2)) \
                    | (GPIO_OSPEEDR_10MHz << (13*2));
    
    // Assert Port H as HiZ
#   if 0    //BOARD_FEATURE_HFXTAL
    GPIOH->MODER    = (GPIO_MODER_IN << (0*2))
                    | (GPIO_MODER_OUT << (1*2));
#   else
    GPIOH->MODER    = (GPIO_MODER_IN << (0*2))
                    | (GPIO_MODER_IN << (1*2));
#   endif
    
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


static inline void BOARD_RFANT_OFF(void) {
/// Clear PA1, PC1, PC2 and change to Analog-In
    ot_u32 ant_moder;
    GPIOA->BSRR = ((1<<1) << 16);
    GPIOC->BSRR = (((1<<1) | (1<<2)) << 16);
    
    ant_moder       = GPIOA->MODER;
    ant_moder      &= ~((3 << (1*2)));
    ant_moder      |= ((3 << (1*2)));
    GPIOA->MODER    = ant_moder;
    
    ant_moder       = GPIOC->MODER;
    ant_moder      &= ~((3 << (1*2)) | (3 << (2*2)));
    ant_moder      |= ((3 << (1*2)) | (3 << (2*2)));
    GPIOC->MODER    = ant_moder;
}


static inline void BOARD_RFANT_ON(void) {
/// Clear PA1, PC1, PC2 and change to Analog-In
    ot_u32 ant_moder;
    GPIOA->BSRR = ((1<<1) << 16);
    GPIOC->BSRR = (((1<<1) | (1<<2)) << 16);
    
    ant_moder       = GPIOA->MODER;
    ant_moder      &= ~((3 << (1*2)));
    ant_moder      |= ((1 << (1*2)));
    GPIOA->MODER    = ant_moder;
    
    ant_moder       = GPIOC->MODER;
    ant_moder      &= ~((3 << (1*2)) | (3 << (2*2)));
    ant_moder      |= ((1 << (1*2)) | (1 << (2*2)));
    GPIOC->MODER    = ant_moder;
}

static inline void BOARD_RFANT_TX(ot_bool use_boost) {
    GPIOC->BSRR = ((1<<2) >> (ot_u32)use_boost);
}

static inline void BOARD_RFANT_RX(void) {
    GPIOA->BSRR = (1<<1);
}



static inline void BOARD_RFSPI_CLKON(void) {
#ifdef __USE_RADIO
    ot_u32 spi_moder;
    spi_moder   = GPIOA->MODER;
    spi_moder  &= ~((3 << (BOARD_RFSPI_MISOPINNUM*2)) \
                  | (3 << (BOARD_RFSPI_MOSIPINNUM*2)) );
    spi_moder  |= (GPIO_MODER_ALT << (BOARD_RFSPI_MISOPINNUM*2)) \
                | (GPIO_MODER_ALT << (BOARD_RFSPI_MOSIPINNUM*2));
    GPIOA->MODER = spi_moder;
    
    spi_moder   = GPIOB->MODER;
    spi_moder  &= ~(3 << (BOARD_RFSPI_SCLKPINNUM*2));
    spi_moder  |= (GPIO_MODER_ALT << (BOARD_RFSPI_SCLKPINNUM*2));
    GPIOB->MODER = spi_moder;
#endif
}

static inline void BOARD_RFSPI_CLKOFF(void) {
#ifdef __USE_RADIO
    ot_u32 spi_moder;
    spi_moder   = GPIOA->MODER;
    spi_moder  &= ~((3 << (BOARD_RFSPI_MISOPINNUM*2)) \
                  | (3 << (BOARD_RFSPI_MOSIPINNUM*2)) );
    spi_moder  |= (GPIO_MODER_IN << (BOARD_RFSPI_MISOPINNUM*2)) \
                | (GPIO_MODER_OUT << (BOARD_RFSPI_MOSIPINNUM*2));
    GPIOA->MODER = spi_moder;
    
    spi_moder   = GPIOB->MODER;
    spi_moder  &= ~(3 << (BOARD_RFSPI_SCLKPINNUM*2));
    spi_moder  |= (GPIO_MODER_OUT << (BOARD_RFSPI_SCLKPINNUM*2));
    GPIOB->MODER = spi_moder;
    
#endif
}



#include <platform/timers.h>    // for systim_stop_clocker()
static inline void BOARD_STOP(ot_int code) {
/// "code" comes from sys_sig_powerdown, but it is usually 0-3.
/// - code = 0 or 1: Don't go to STOP mode at all, this function shouldn't be called.
/// - code = 2: Keep ports alive that have peripherals that can be active in STOP
/// - code = 3: Shut down all ports, only
/// Those are the only modes that should call this inline function.

#   define _RF_ACTIVE_STOPFLAGS (PWR_CR_LPSDSR | PWR_CR_CSBF)
#   define _RF_OFF_STOPFLAGS    (PWR_CR_LPSDSR | PWR_CR_FWU | PWR_CR_ULP | PWR_CR_CSBF)

    static const ot_u16 stop_flags[2] = {  
        _RF_ACTIVE_STOPFLAGS, _RF_OFF_STOPFLAGS };
    
    static const ot_u32 rcc_flags[2] = {
        _GPIOCLK_LPRF, _GPIOCLK_LP };
        //_GPIOCLK_N, 0 };
    
    ot_u16 scratch;
    code &= 1;
    
    // Shut down unnecessary ports & kill SysTick.  Systick is the devil.
    RCC->IOPENR     = rcc_flags[code];
    SysTick->CTRL   = 0;
    
    // Prepare SCB->SCR register for DeepSleep instead of regular Sleep.
    SCB->SCR       |= SCB_SCR_SLEEPDEEP_Msk;
    
    // Prepare PWR->CR register for Stop Mode
    scratch         = PWR->CR;
    scratch        &= ~(PWR_CR_DBP | PWR_CR_PDDS | PWR_CR_LPSDSR);
    scratch        |= stop_flags[code];
    PWR->CR         = scratch;
    
    // Clear any EXTIs -- too late for them now
    EXTI->PR        = 0;
    
    // Enable Interrupts and send "Wait For Interrupt" Cortex-M instruction
    platform_enable_interrupts();
    __WFI();
    
    // ---- Device is in STOP mode here ---- //
    
    // On Wakeup (from STOP) clear flags, re-enable backup register area, 
    // immediately reset SLEEPDEEP bit, and Re-enable Ports
    PWR->CR        |= (PWR_CR_DBP | PWR_CR_CWUF | PWR_CR_CSBF);
    SCB->SCR       &= ~((ot_u32)SCB_SCR_SLEEPDEEP_Msk);
    RCC->IOPENR     = _IOPENR_RUNTIME;

#   undef _RF_ACTIVE_STOPFLAGS 
#   undef _RF_OFF_STOPFLAGS 
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
#define BOARD_RADIO_EXTI1_ISR()     sx127x_irq1_isr()
#define BOARD_RADIO_EXTI2_ISR();
#define BOARD_RADIO_EXTI3_ISR();  
#define BOARD_RADIO_EXTI4_ISR()     sx127x_irq0_isr()
#define BOARD_RADIO_EXTI5_ISR();
#define BOARD_RADIO_EXTI6_ISR();     
#define BOARD_RADIO_EXTI7_ISR();     
#define BOARD_RADIO_EXTI8_ISR();
#define BOARD_RADIO_EXTI9_ISR();
#define BOARD_RADIO_EXTI10_ISR();
#define BOARD_RADIO_EXTI11_ISR();
#define BOARD_RADIO_EXTI12_ISR();
#define BOARD_RADIO_EXTI13_ISR()    sx127x_irq3_isr()
#define BOARD_RADIO_EXTI14_ISR();
#define BOARD_RADIO_EXTI15_ISR();


///@todo Create a more intelligent setup that knows how to use the UART
///@note BOARD Macros for Com module interrupt vectoring.  Connect these to
///      the Com interface driver you are using.  Check the schematic of your
///      board to see where the Com IRQ lines are routed. 
#if (MCU_CONFIG(MPIPEUART) && OT_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE) && BOARD_FEATURE(MPIPE_BREAK))
#   define _UART_RXSYNC_ISR()       mpipe_rxsync_isr()
#else
#   define _UART_RXSYNC_ISR()       ;
#endif

#if defined(__UBX_GNSS__)
#   define _GNSS_RXSYNC_ISR()       ubxdrv_rxsync_isr()
#else
#   define _GNSS_RXSYNC_ISR()       ;
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
#define BOARD_COM_EXTI10_ISR()  _GNSS_RXSYNC_ISR()
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

// Default is 2^3 oversampling.  If you want to change defaults, you can, but
// be careful.
#define OT_GPTIM_SHIFT      MCU_PARAM_GPTIM_SHIFT
#define OT_GPTIM_OVERSAMPLE MCU_PARAM_GPTIM_OVERSAMPLE

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



// Primary LED triggers
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
#define OT_TRIG4_PORT       BOARD_LEDB_PORT
#define OT_TRIG4_PINNUM     BOARD_LEDB_PINNUM
#define OT_TRIG4_PIN        BOARD_LEDB_PIN
#define OT_TRIG4_POLARITY   BOARD_LEDB_POLARITY

// Secondary pin triggers
#define OT_TRIG5_PORT       BOARD_TRIG5_PORT
#define OT_TRIG5_PINNUM     BOARD_TRIG5_PINNUM
#define OT_TRIG5_PIN        BOARD_TRIG5_PIN
#define OT_TRIG5_POLARITY   1
#define OT_TRIG6_PORT       BOARD_TRIG6_PORT
#define OT_TRIG6_PINNUM     BOARD_TRIG6_PINNUM
#define OT_TRIG6_PIN        BOARD_TRIG6_PIN
#define OT_TRIG6_POLARITY   1



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
static inline void BOARD_led4_on(void)      { OT_TRIG4_ON(); }
static inline void BOARD_led4_off(void)     { OT_TRIG4_OFF(); }
static inline void BOARD_led4_toggle(void)  { OT_TRIG4_TOG(); }


#ifndef __ISR_EXTI2
#   define __ISR_EXTI2
#endif
#define OT_SWITCH1_ISR      platform_isr_exti2
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
#   ifndef __ISR_USART2
#       define __ISR_USART2
#   endif

#   if (defined(__USE_DMA1_CHAN6) || defined(__USE_DMA1_CHAN7))
#       error "MPIPE UART is UART2 is set-up on DMA channels 6 and 7, but they are already configured elsewhere."
#   endif
#   define MPIPE_UART           USART2
#   define MPIPE_DMA_RXCHAN_ID  6
#   define MPIPE_DMA_TXCHAN_ID  7
#   define __USE_DMA1_CHAN6
#   define __USE_DMA1_CHAN7

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
#       if (defined(__USE_DMA1_CHAN4) || defined(__USE_DMA1_CHAN5))
#           error "MPIPE is on I2C and set-up on DMA channels 4 and 5, but they are already configured elsewhere."
#       endif
#       define MPIPE_DMANUM         1
#       define MPIPE_DMA            DMA1
#       define MPIPE_DMA_RXCHAN_ID  5
#       define MPIPE_DMA_TXCHAN_ID  4
#       define __USE_DMA1_CHAN5
#       define __USE_DMA1_CHAN4
#   endif
#endif
#endif




/** UBX GNSS UART Setup forBR>
  * ========================================================================<BR>
  */
#if (OT_FEATURE(GNSS) && BOARD_FEATURE(UBX_GNSS))
#   define GNSS_DMANUM         1
#   define GNSS_DMA            DMA1
#   define GNSS_UART_ID        BOARD_EXTUART_ID
#   define GNSS_UART_PORTNUM   BOARD_EXTUART_PORTNUM
#   define GNSS_UART_PORT      BOARD_EXTUART_PORT
#   define GNSS_UART_RXPIN     BOARD_EXTUART_RXPIN
#   define GNSS_UART_TXPIN     BOARD_EXTUART_TXPIN
#   define GNSS_UART_PINS      (GNSS_UART_RXPIN | GNSS_UART_TXPIN)
#   if (GNSS_UART_ID != 1)
#       error "GNSS UART must be on USART1 for this board."
#   endif
#   ifndef __ISR_USART1
#       define __ISR_USART1
#   endif
#   define GNSS_UART           USART1
#   if (defined(__USE_DMA1_CHAN4) || defined(__USE_DMA1_CHAN5))
#       error "GNSS UART is UART1 is set-up on DMA channels 4 and 5, but they are already configured elsewhere."
#   endif
#   define GNSS_DMA_RXCHAN_ID  5
#   define GNSS_DMA_TXCHAN_ID  4
#   define __USE_DMA1_CHAN4
#   define __USE_DMA1_CHAN5
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
#define RADIO_SPICLK_PORT           GPIOB
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

#if defined(__UBX_GNSS__)
#   include <io/ubx_gnss.h>
#elif defined(__NULL_GNSS__)
#   include <io/gnss_null/interface.h>
#endif


#endif
