/* Copyright 2009-2012 JP Norair
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
  */
/**
  * @file       /board/stm32f10x/board_opennode.h
  * @author     JP Norair
  * @version    V1.0
  * @date       7 August 2011
  * @brief      Board Configuration for OpenNode
  * @ingroup    Platform
  *
  * Do not include this file, include OT_platform.h
  ******************************************************************************
  */
  

#ifndef __board_OPENNODE_H
#define __board_OPENNODE_H

#include <app/build_config.h>
#include "platform_STM32F10x.h"
#include "radio_cc1101.h"





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




/** Supported [Known] STM32 devices used by OpenNode <BR>
  * ========================================================================<BR>
  * STM32F103CBT6 = "Medium density performance line" = STM32F10X_MD
  */

#ifndef STM32F10X_MD
#   define STM32F10X_MD
#endif

#ifndef MCU_STM32F103CBT6
#   define MCU_STM32F103CBT6
#endif

#if !defined(MCU_STM32F103CBT6)
#   error "A supported MCU was not defined (need for linker constants)"
#endif




/** Additional RF Front End Parameters and Settings <BR>
  * ========================================================================<BR>
  */
#define RF_PARAM_BAND   433
#define RF_HDB_ATTEN    6       //Half dB attenuation (units = 0.5dB), used to scale TX power
#define RF_RSSI_OFFSET  3       //Offset applied to RSSI calculation





/** MCU Feature settings      <BR>
  * ========================================================================<BR>
  * Implemented capabilities of the STM32F103CBT6
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL       // FEATURE                  AVAILABILITY
#define MCU_FEATURE_CRC                 DISABLED                // CCITT CRC16              Low
#define MCU_FEATURE_AES128              DISABLED                // AES128 engine            Moderate
#define MCU_FEATURE_ECC                 DISABLED                // ECC engine               Low
#define MCU_FEATURE_EEPROM              DISABLED                // No EEPROM on STM32F10x

#define MCU_FEATURE_RADIODMA            DISABLED
#define MCU_FEATURE_RADIODMA_TXBYTES    0
#define MCU_FEATURE_RADIODMA_RXBYTES    0

#define MCU_FEATURE_MAPEEPROM           DISABLED                
#define MCU_FEATURE_MPIPECDC            ENABLED
#define MCU_FEATURE_MPIPEDMA            (ENABLED && (MCU_FEATURE_MPIPECDC == DISABLED))

///@todo include later
#define MCU_FEATURE_MEMCPYDMA           DISABLED






/** Board-based Feature Settings <BR>
  * ========================================================================<BR>
  * Notes apart from the obvious:  
  *
  * 1. The SPI bus to the CC1101 MUST be less than 6.5 MHz.  It should be as
  *    close to 6.5 MHz as possible.  STM32 SPI has mandatory prescaler of at
  *    least 2, so optimal APB speed is 12 MHz
  *
  * 2. If you use a normal Timer for the GPTIM (not RTC-based timer), the input
  *    bus should be 48 MHz.  48MHz/46875 = 1024 Hz.  GPTIM should have 1024 Hz,
  *    or a power-of-2 multiple (e.g. 2048, 4096, 8192, 16384, 32768)
  * 
  * 3. The 3 LEDs (R,Y,G) are normal-biased.
  */
#define BOARD_FEATURE(VAL)              BOARD_FEATURE_##VAL
#define BOARD_FEATURE_USBCONVERTER      ENABLED                 // Is UART connected via USB converter?
#define BOARD_FEATURE_MPIPE_QMGMT		ENABLED
#define BOARD_FEATURE_LFXTAL            ENABLED                 // LF XTAL used as Clock source
#define BOARD_FEATURE_HFXTAL            DISABLED                // HF XTAL used as Clock source
#define BOARD_FEATURE_INVERT_TRIG1      DISABLED
#define BOARD_FEATURE_INVERT_TRIG2      DISABLED

#define BOARD_UKEY_PORT                 GPIOA
#define BOARD_UKEY_PIN                  GPIO_Pin_1
#define BOARD_UKEY_POLARITY             0

#define BOARD_PARAM(VAL)                BOARD_PARAM_##VAL
#define BOARD_PARAM_LFHz                32768
#define BOARD_PARAM_LFtol               0.00002
#define BOARD_PARAM_HFHz                8000000
#define BOARD_PARAM_HFmult              6                       // DCO = HFHz * HFmult
#define BOARD_PARAM_HFtol               0.00002
#define BOARD_PARAM_AHBDIV              1                       // Main Bus Clock Div
#define BOARD_PARAM_APB1DIV             4                       // Slow Peripheral Bus Clock Div (12 MHz)
#define BOARD_PARAM_APB2DIV             1                       // Fast Peripheral Bus Clock Div

#define BOARD_USB_VBUS_PORT             GPIOA
#define BOARD_USB_VBUS_PIN              GPIO_Pin_10
#define BOARD_USB_DM_PORT               GPIOA
#define BOARD_USB_DM_PIN                GPIO_Pin_11
#define BOARD_USB_DP_PORT               GPIOA
#define BOARD_USB_DP_PIN                GPIO_Pin_12

#define BOARD_LEDR_PORTNUM              'B'
#define BOARD_LEDR_PORT                 GPIOB
#define BOARD_LEDR_PIN                  GPIO_Pin_0
#define BOARD_LEDR_POLARITY             1
#define BOARD_LEDR_HIDRIVE              0
#define BOARD_LEDY_PORTNUM              'B'
#define BOARD_LEDY_PORT                 GPIOB
#define BOARD_LEDY_PIN                  GPIO_Pin_2
#define BOARD_LEDY_POLARITY             1
#define BOARD_LEDY_HIDRIVE              0
#define BOARD_LEDG_PORTNUM              'B'
#define BOARD_LEDG_PORT                 GPIOB
#define BOARD_LEDG_PIN                  GPIO_Pin_3
#define BOARD_LEDG_POLARITY             1
#define BOARD_LEDG_HIDRIVE              0

#define BOARD_EEPROM_I2C                I2C2
#define BOARD_EEPROM_I2C_PORT           GPIOB
#define BOARD_EEPROM_I2C_SCL            GPIO_Pin_10
#define BOARD_EEPROM_I2C_SDA            GPIO_Pin_11

#define BOARD_ACCEL_I2C                 BOARD_EEPROM_I2C
#define BOARD_ACCEL_I2C_PORT            BOARD_EEPROM_I2C_PORT
#define BOARD_ACCEL_I2C_SCL             BOARD_EEPROM_I2C_SCL
#define BOARD_ACCEL_I2C_SDA             BOARD_EEPROM_I2C_SDA
#define BOARD_ACCEL_IRQ0_PORT           GPIOA
#define BOARD_ACCEL_IRQ0_PIN            GPIO_Pin_0

#define BOARD_RADIO_SPINUM              2
#define BOARD_RADIO_SPI                 SPI2
#define BOARD_RADIO_SPI_PORT            GPIOB
#define BOARD_RADIO_SPI_NSS             GPIO_Pin_12
#define BOARD_RADIO_SPI_SCK             GPIO_Pin_13
#define BOARD_RADIO_SPI_MISO            GPIO_Pin_14
#define BOARD_RADIO_SPI_MOSI            GPIO_Pin_15
#define BOARD_RADIO_IRQ0_PORTNUM        'A'
#define BOARD_RADIO_IRQ0_PORT           GPIOA
#define BOARD_RADIO_IRQ0_PIN            GPIO_Pin_9
#define BOARD_RADIO_IRQ2_PORTNUM        'A'
#define BOARD_RADIO_IRQ2_PORT           GPIOA
#define BOARD_RADIO_IRQ2_PIN            GPIO_Pin_8

#define BOARD_CANBUS                    CAN1
#define BOARD_CANBUS_PORT               GPIOB
#define BOARD_CANBUS_RX                 GPIO_Pin_8
#define BOARD_CANBUS_TX                 GPIO_Pin_9

#define BOARD_UARTBUS                   USART2
#define BOARD_UARTBUS_PORT              GPIOA
#define BOARD_UARTBUS_TX                GPIO_Pin_2
#define BOARD_UARTBUS_RX                GPIO_Pin_3

#define BOARD_GPIO0_PORT                GPIOA
#define BOARD_GPIO0_PIN                 GPIO_Pin_4
#define BOARD_GPIO1_PORT                GPIOA
#define BOARD_GPIO1_PIN                 GPIO_Pin_5
#define BOARD_GPIO2_PORT                GPIOA
#define BOARD_GPIO2_PIN                 GPIO_Pin_6
#define BOARD_GPIO3_PORT                GPIOA
#define BOARD_GPIO3_PIN                 GPIO_Pin_7


OT_INLINE_H BOARD_RADIO_SPI_PORTCONF() {
    // Clear Pins 12, 13, 14, 15
    // Then setup MOSI->AFOUT, MISO->IN_FLOATING, SCK->AFOUT, CS->OUT
    BOARD_RADIO_SPI_PORT->CRH  &= 0x0000FFFF;
    BOARD_RADIO_SPI_PORT->CRH  |= (b1011 << 28) | (b0100 << 24) | (b1011 << 20) | (b0011 << 16);
}




/** Peripherals to use for Debugging <BR>
  * ========================================================================<BR>
  * At the moment, these must be entered manually
  */
#define OT_DEBUG_PERIPHERALS    (DBGMCU_SLEEP    | \
                                DBGMCU_STOP      | \
                                DBGMCU_STANDBY   | \
                                DBGMCU_IWDG_STOP | \
                                DBGMCU_WWDG_STOP | \
                                DBGMCU_TIM2_STOP)






/** Platform Memory Configuration <BR>
  * ========================================================================<BR>
  * OpenTag needs to know where it can put Nonvolatile memory (file system) and
  * how much space it can allocate for filesystem.
  */

#if defined (MCU_STM32F103CBT6)
#   define SRAM_START_ADDR      0x20000000
#   define SRAM_AVAILABLE            (20*1024)
#   define EEPROM_START_ADDR    0
#   define EEPROM_AVAILABLE          0
#   define FLASH_START_ADDR     0x08000000
#   define FLASH_PAGE_SIZE      (1024)
#   define FLASH_NUM_PAGES      128
#   define FLASH_WORD_BYTES     2
#   define FLASH_WORD_BITS      (FLASH_WORD_BYTES*8)
#endif

#define FLASH_FS_ALLOC      (8*FLASH_PAGE_SIZE)                                 //allocating 8KB total
//#define FLASH_FS_ADDR     (64*1024 - FLASH_FS_ALLOC)                          //For 64KB limited RIDE
#define FLASH_FS_ADDR       (FLASH_NUM_PAGES*FLASH_PAGE_SIZE - FLASH_FS_ALLOC)  //For unlimited toolchain
#define FLASH_FS_FALLOWS    3





/** Note: Clocking for this platform      <BR>
  * ========================================================================<BR>
  * The STM32 can be set at different clock rates.  I generally recommend using
  * the highest clock speed that can be used without activating the PLL.  There
  * is some marginal benefit as well if you keep wait states to 0.
  *
  * Flash Wait States per Clock speed:                              <BR>
  * STM32F: Vnom:   0 < clock <= 24MHz :    0 wait states           <BR>
  * STM32F: Vnom:   24 < clock <= 48MHz :   1 wait state            <BR>
  * STM32F: Vnom:   48 < clock <= 72MHz :   2 wait states           <BR>
  * STM32L: 1.2V:   0 < clock <= 2MHz :     0 wait states           <BR>
  * STM32L: 1.2V:   2 < clock <= 4MHz :     1 wait state            <BR>
  * STM32L: 1.5V:   0 < clock <= 8MHz :     0 wait states           <BR>
  * STM32L: 1.5V:   8 < clock <= 16MHz :    1 wait state            <BR>
  * STM32L: 1.8V:   0 < clock <= 16MHz :    0 wait states           <BR>
  * STM32L: 1.8V:   16 < clock <= 32MHz :   1 wait state            <BR>
  *
  */
#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_LFXTALHz              BOARD_PARAM_LFHz
#define MCU_PARAM_LFXTALtol             BOARD_PARAM_LFtol
//#define MCU_PARAM_LFOSCHz               BOARD_PARAM_LFHz
//#define MCU_PARAM_LFOSCtol              0.02
#define MCU_PARAM_XTALHz                (BOARD_PARAM_HFHz*BOARD_PARAM_HFmult)
#define MCU_PARAM_XTALtol               BOARD_PARAM_HFtol
//#define MCU_PARAM_OSCHz               (BOARD_PARAM_HFHz*BOARD_PARAM_HFmult)
//#define MCU_PARAM_OSCtol                0.02


#define PLATFORM_PARAM_AHBDIV       BOARD_PARAM_AHBDIV
#define PLATFORM_PARAM_APB1DIV      BOARD_PARAM_APB1DIV
#define PLATFORM_PARAM_APB2DIV      BOARD_PARAM_APB2DIV

#if defined(MCU_PARAM_LFXTALHz)
#   define PLATFORM_LSCLOCK_HZ      MCU_PARAM_LFXTALHz
#   define PLATFORM_LSCLOCK_ERROR   MCU_PARAM_LFXTALtol
#elif defined(MCU_PARAM_LFOSCHz)
#   define PLATFORM_LSCLOCK_HZ      MCU_PARAM_LFOSCHz
#   define PLATFORM_LSCLOCK_ERROR   MCU_PARAM_LFOSCtol
#endif

#if defined(MCU_PARAM_XTALHz)
#   define PLATFORM_HSCLOCK_HZ      MCU_PARAM_XTALHz
#   define PLATFORM_HSCLOCK_ERROR   MCU_PARAM_XTALtol
#elif defined(MCU_PARAM_OSCHz)
#   define PLATFORM_HSCLOCK_HZ      MCU_PARAM_OSCHz
#   define PLATFORM_HSCLOCK_ERROR   MCU_PARAM_OSCtol
#endif




/** Peripheral definitions for this platform <BR>
  * ========================================================================<BR>
  * OT_GPTIM:   General Purpose Timer used by OpenTag kernel (best is TIM9) <BR>
  * OT_TRIG:    Optional test trigger usable in OpenTag apps (often LEDs)   <BR>
  * MPIPE:      USB or UART to use for the MPipe                            <BR>
  * RADIO_SPI:  SPI used to communicate with RF core (best is SPI1)         <BR>
  * RADIO_IRQx: GPIO pin to use for radio IRQ signal (best is PA0-3)        <BR>
  * RADIO_DMA:  The SPIbus uses a DMA to automate the transfer management.
  *             There are some limits on which DMAs can be used on each SPI. <BR>
  * <PRE>
  * | RADIO_SPI | RADIO_DMA | RX Chan | TX Chan |
  * +-----------+-----------+---------+---------+
  * |   SPI1    |   DMA1    | Chan 2  | Chan 3  |
  * |   SPI2    |   DMA1    | Chan 4  | Chan 5  |
  * |   SPI3    |   DMA2    | Chan 1  | Chan 2  |
  * </PRE>
  *
  */

#define OT_GPTIM_INDEX      2                           // GPTIM INDEX: TIMx, 0 for using Systick, or -1 for RTC
#define OT_GPTIM            TIM2
#define OT_GPTIM_IRQn       TIM2_IRQn
#define OT_GPTIM_CLOCK      PLATFORM_HSCLOCK_HZ         // Alternatively: PLATFORM_LSCLOCK_HZ
#define OT_GPTIM_RES        1024

#if (OT_GPTIM_CLOCK == PLATFORM_LSCLOCK_HZ)
#   define OT_GPIM_ERROR    PLATFORM_LSCLOCK_ERROR
#else
#   define OT_GPIM_ERROR    PLATFORM_HSCLOCK_ERROR
#endif
#define OT_GPTIM_ERRDIV     (1/OT_GPIM_ERROR)
#define TI_TO_CLK(VAL)      ((OT_GPTIM_RES/1024)*VAL)
#define CLK_TO_TI(VAL)      (VAL/(OT_GPTIM_RES/1024))



/** Triggers (generally Green+Orange LEDs) <BR>
  * ========================================================================<BR>
  */
#define OT_TRIG1_PORTNUM    BOARD_LEDG_PORTNUM
#define OT_TRIG1_PORT       BOARD_LEDG_PORT
#define OT_TRIG1_PIN        BOARD_LEDG_PIN
#define OT_TRIG1_HIDRIVE    BOARD_LEDG_HIDRIVE
#define OT_TRIG2_PORTNUM    BOARD_LEDY_PORTNUM
#define OT_TRIG2_PORT       BOARD_LEDY_PORT
#define OT_TRIG2_PIN        BOARD_LEDY_PIN
#define OT_TRIG2_HIDRIVE    BOARD_LEDY_HIDRIVE


/** Radio IRQ pins for CC1101 (kind of laborious) <BR>
  * ========================================================================<BR>
  */
#define RADIO_IRQ0_PORT     BOARD_RADIO_IRQ0_PORT
#define RADIO_IRQ0_PIN      BOARD_RADIO_IRQ0_PIN
#define RADIO_IRQ2_PORT     BOARD_RADIO_IRQ2_PORT
#define RADIO_IRQ2_PIN      BOARD_RADIO_IRQ2_PIN

#if (BOARD_RADIO_IRQ0_PORTNUM == 'A')
#   define RADIO_IRQ0_SRCPORT   GPIO_PortSourceGPIOA
#elif (BOARD_RADIO_IRQ0_PORTNUM == 'B')
#   define RADIO_IRQ0_SRCPORT   GPIO_PortSourceGPIOB
#elif (BOARD_RADIO_IRQ0_PORTNUM == 'C')
#   define RADIO_IRQ0_SRCPORT   GPIO_PortSourceGPIOC
#elif (BOARD_RADIO_IRQ0_PORTNUM == 'D')
#   define RADIO_IRQ0_SRCPORT   GPIO_PortSourceGPIOD
#endif

#if (BOARD_RADIO_IRQ2_PORTNUM == 'A')
#   define RADIO_IRQ2_SRCPORT   GPIO_PortSourceGPIOA
#elif (BOARD_RADIO_IRQ0_PORTNUM == 'B')
#   define RADIO_IRQ2_SRCPORT   GPIO_PortSourceGPIOB
#elif (BOARD_RADIO_IRQ0_PORTNUM == 'C')
#   define RADIO_IRQ2_SRCPORT   GPIO_PortSourceGPIOC
#elif (BOARD_RADIO_IRQ0_PORTNUM == 'D')
#   define RADIO_IRQ2_SRCPORT   GPIO_PortSourceGPIOD
#endif

#if (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_0)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource0
#   define RADIO_IRQ0_SRCLINE  0
#   define RADIO_IRQ0_IRQn     EXTI0_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_1)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource1
#   define RADIO_IRQ0_SRCLINE  1
#   define RADIO_IRQ0_IRQn     EXTI1_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_2)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource2
#   define RADIO_IRQ0_SRCLINE  2
#   define RADIO_IRQ0_IRQn     EXTI2_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_3)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource3
#   define RADIO_IRQ0_SRCLINE  3
#   define RADIO_IRQ0_IRQn     EXTI3_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_4)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource4
#   define RADIO_IRQ0_SRCLINE  4
#   define RADIO_IRQ0_IRQn     EXTI4_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_5)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource5
#   define RADIO_IRQ0_SRCLINE  5
#   define RADIO_IRQ0_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_6)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource6
#   define RADIO_IRQ0_SRCLINE  6
#   define RADIO_IRQ0_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_7)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource7
#   define RADIO_IRQ0_SRCLINE  7
#   define RADIO_IRQ0_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_8)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource8
#   define RADIO_IRQ0_SRCLINE  8
#   define RADIO_IRQ0_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_9)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource9
#   define RADIO_IRQ0_SRCLINE  9
#   define RADIO_IRQ0_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_10)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource10
#   define RADIO_IRQ0_SRCLINE  10
#   define RADIO_IRQ0_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_11)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource11
#   define RADIO_IRQ0_SRCLINE  11
#   define RADIO_IRQ0_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_12)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource12
#   define RADIO_IRQ0_SRCLINE  12
#   define RADIO_IRQ0_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_13)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource13
#   define RADIO_IRQ0_SRCLINE  13
#   define RADIO_IRQ0_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_14)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource14
#   define RADIO_IRQ0_SRCLINE  14
#   define RADIO_IRQ0_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ0_PIN == GPIO_Pin_15)
#   define RADIO_IRQ0_SRCPIN   GPIO_PinSource15
#   define RADIO_IRQ0_SRCLINE  15
#   define RADIO_IRQ0_IRQn     EXTI15_10_IRQn
#endif

#if (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_0)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource0
#   define RADIO_IRQ2_SRCLINE  0
#   define RADIO_IRQ2_IRQn     EXTI0_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_1)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource1
#   define RADIO_IRQ2_SRCLINE  1
#   define RADIO_IRQ2_IRQn     EXTI1_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_2)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource2
#   define RADIO_IRQ2_SRCLINE  2
#   define RADIO_IRQ2_IRQn     EXTI2_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_3)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource3
#   define RADIO_IRQ2_SRCLINE  3
#   define RADIO_IRQ2_IRQn     EXTI3_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_4)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource4
#   define RADIO_IRQ2_SRCLINE  4
#   define RADIO_IRQ2_IRQn     EXTI4_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_5)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource5
#   define RADIO_IRQ2_SRCLINE  5
#   define RADIO_IRQ2_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_6)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource6
#   define RADIO_IRQ2_SRCLINE  6
#   define RADIO_IRQ2_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_7)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource7
#   define RADIO_IRQ2_SRCLINE  7
#   define RADIO_IRQ2_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_8)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource8
#   define RADIO_IRQ2_SRCLINE  8
#   define RADIO_IRQ2_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_9)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource9
#   define RADIO_IRQ2_SRCLINE  9
#   define RADIO_IRQ2_IRQn     EXTI9_5_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_10)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource10
#   define RADIO_IRQ2_SRCLINE  10
#   define RADIO_IRQ2_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_11)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource11
#   define RADIO_IRQ2_SRCLINE  11
#   define RADIO_IRQ2_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_12)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource12
#   define RADIO_IRQ2_SRCLINE  12
#   define RADIO_IRQ2_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_13)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource13
#   define RADIO_IRQ2_SRCLINE  13
#   define RADIO_IRQ2_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_14)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource14
#   define RADIO_IRQ2_SRCLINE  14
#   define RADIO_IRQ2_IRQn     EXTI15_10_IRQn
#elif (BOARD_RADIO_IRQ2_PIN == GPIO_Pin_15)
#   define RADIO_IRQ2_SRCPIN   GPIO_PinSource15
#   define RADIO_IRQ2_SRCLINE  15
#   define RADIO_IRQ2_IRQn     EXTI15_10_IRQn
#endif




/** Radio DMA only used when MCU_RADIODMA == ENABLED (not always) <BR>
  * ========================================================================<BR>
  */
#define RADIO_DMA_RXINT  	DMA1_IT_TC2
#define RADIO_DMA_TXINT  	DMA1_IT_TC3


/** Radio SPI for CC1101, with connection to optional DMA <BR>
  * ========================================================================<BR>
  */
#define RADIO_SPI_INDEX     BOARD_RADIO_SPINUM
#define RADIO_SPI           BOARD_RADIO_SPI
#define RADIO_SPI_REMAP     DISABLED

#define RADIO_SPICS_PORT    BOARD_RADIO_SPIPORT
#define RADIO_SPICS_PIN     BOARD_RADIO_SPINSS
#define RADIO_SPISCK_PORT   BOARD_RADIO_SPIPORT
#define RADIO_SPISCK_PIN    BOARD_RADIO_SPISCK
#define RADIO_SPIMISO_PORT  BOARD_RADIO_SPIPORT
#define RADIO_SPIMISO_PIN   BOARD_RADIO_SPIMISO
#define RADIO_SPIMOSI_PORT  BOARD_RADIO_SPIPORT
#define RADIO_SPIMOSI_PIN   BOARD_RADIO_SPIMOSI

#if (BOARD_RADIO_SPINUM == 1)
#   define RADIO_SPI_IRQn      SPI1_IRQn
#   define RADIO_SPI_GPIO      GPIO_AF_SPI1
#   define RADIO_DMA_INDEX     1
#   define RADIO_DMA_RXINDEX   12
#   define RADIO_DMA_TXINDEX   13
#   define RADIO_DMA           DMA1
#   define RADIO_DMA_RXCHAN    DMA1_Channel2
#   define RADIO_DMA_RXIRQn    DMA1_Channel2_IRQn
#   define RADIO_DMA_TXCHAN    DMA1_Channel3
#   define RADIO_DMA_TXIRQn    DMA1_Channel3_IRQn

#elif (BOARD_RADIO_SPINUM == 2)
#   define RADIO_SPI_IRQn      SPI2_IRQn
#   define RADIO_SPI_GPIO      GPIO_AF_SPI2
#   define RADIO_DMA_INDEX     2
#   define RADIO_DMA_RXINDEX   14
#   define RADIO_DMA_TXINDEX   15
#   define RADIO_DMA           DMA1
#   define RADIO_DMA_RXCHAN    DMA1_Channel4
#   define RADIO_DMA_RXIRQn    DMA1_Channel4_IRQn
#   define RADIO_DMA_TXCHAN    DMA1_Channel5
#   define RADIO_DMA_TXIRQn    DMA1_Channel5_IRQn

#elif (BOARD_RADIO_SPINUM == 3)
#   define RADIO_SPI_IRQn      SPI3_IRQn
#   define RADIO_SPI_GPIO      GPIO_AF_SPI3
#   define RADIO_DMA_INDEX     3
#   define RADIO_DMA_RXINDEX   21
#   define RADIO_DMA_TXINDEX   22
#   define RADIO_DMA           DMA2
#   define RADIO_DMA_RXCHAN    DMA2_Channel1
#   define RADIO_DMA_RXIRQn    DMA2_Channel1_IRQn
#   define RADIO_DMA_TXCHAN    DMA2_Channel2
#   define RADIO_DMA_TXIRQn    DMA2_Channel2_IRQn

#else
#   error "RADIO SPI must be on SPI 1, 2, or 3"
#endif



/** Radio Clock Speed Check for CC1101 <BR>
  * ========================================================================<BR>
  * CC1101 SPI must be less than 6.5 MHz
  */
#define RADIO_SPI_CLKSRC    (PLATFORM_HSCLOCK_HZ / \
                             (1 + ((RADIO_SPI_INDEX != 1)*(BOARD_PARAM_APB1DIV-1))))
#if (RADIO_SPI_CLKSRC < 13000000)
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_2
#elif (RADIO_SPI_CLKSRC < 26000000)
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_4
#elif (RADIO_SPI_CLKSRC < 52000000)
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_8
#elif (RADIO_SPI_CLKSRC < 108000000)
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_16
#else 
#	define RADIO_SPI_PRESCALER SPI_BaudRatePrescaler_32
#endif





/** MPipe USB-CDC setup <BR>
  * ========================================================================<BR>
  * Clock must be 48MHz or 72MHz (typically it is 48 MHz), and it must use an
  * external crystal (typically 8 MHz).
  */
#if (MCU_PARAM_XTALHz == 48000000)
#   define MPIPE_USBCLK_DIV RCC_USBCLKSource_PLLCLK_Div1
#elif (MCU_PARAM_XTALHz == 72000000)
#   define MPIPE_USBCLK_DIV RCC_USBCLKSource_PLLCLK_1Div5
#else
#   error "To use USB, the system clock MUST be configured to 48 or 72 MHz via the HSE."
#endif

#define MPIPE_USB_IRQn       ((ot_u32)USB_LP_CAN1_RX0_IRQn)
#define MPIPE_USBCLK(VAL)    do { \
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA+RCC_APB2Periph_GPIOB, VAL); \
        RCC_USBCLKConfig(MPIPE_USBCLK_DIV); \
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, VAL); \
        } while(0)

#define MPIPE_USB_INDEX     0
#define MPIPE_USB           USB0
#define MPIPE_REMAP         DISABLED
#define MPIPE_REMAP_TYPE    0           //USB generally is not remappable
#define MPIPE_USBDC_PORT    BOARD_USB_VBUS_PORT
#define MPIPE_USBDC_PIN     BOARD_USB_VBUS_PIN
#define MPIPE_USBDP_PORT    BOARD_USB_DP_PORT
#define MPIPE_USBDP_PIN     BOARD_USB_DP_PIN
#define MPIPE_USBDM_PORT    BOARD_USB_DM_PORT
#define MPIPE_USBDM_PIN     BOARD_USB_DM_PIN





// Uncomment if using UART for MPIPE
/* 
#define MPIPE_UART_INDEX 1
#define MPIPE_UART       USART1
#define MPIPE_UART_IRQn  USART1_IRQn
#define MPIPE_REMAP      DISABLED                 // ENABLED or DISABLED (very platform dependent)
#define MPIPE_REMAP_TYPE GPIO_Remap_USART1       // Check stm32f10x_gpio.c line 492
#define MPIPE_TXD_PORT   GPIOA
#define MPIPE_TXD_PIN    GPIO_Pin_9      //B6 on remap
#define MPIPE_RXD_PORT   GPIOA
#define MPIPE_RXD_PIN    GPIO_Pin_10     //B7 on remap

#define MPIPE_DMA_INDEX  1
#define MPIPE_DMA_RXINDEX 15
#define MPIPE_DMA_TXINDEX 14
#define MPIPE_DMA        DMA1
#define MPIPE_DMA_RXCHAN DMA1_Channel5       // Ch7 on USART2
#define MPIPE_DMA_RXIRQn DMA1_Channel5_IRQn
#define MPIPE_DMA_RXINT  DMA1_IT_GL5
#define MPIPE_DMA_TXCHAN DMA1_Channel4       // Ch6 on USART2
#define MPIPE_DMA_TXIRQn DMA1_Channel4_IRQn
#define MPIPE_DMA_TXINT  DMA1_IT_GL4
*/


#ifndef MPIPE_USB
#   define MPIPE_USB    0
#endif
#ifndef MPIPE_UART
#   define MPIPE_UART   0
#endif
#ifndef MPIPE_SPI
#   define MPIPE_SPI    0
#endif
#ifndef MPIPE_I2C
#   define MPIPE_I2C    0
#endif
#ifndef MPIPE_CAN
#   define MPIPE_CAN    0
#endif








/******* ALL SHIT BELOW HERE IS SUBJECT TO REDEFINITION **********/


/** Flash Memory Setup: 
  * "OTF" means "Open Tag Flash," but if flash is not used, it just means 
  * storage memory.  Unfortunately this does not begin with F.
  */
#define OTF_VWORM_PAGES         (FLASH_FS_ALLOC/FLASH_PAGE_SIZE)
#define OTF_VWORM_FALLOW_PAGES  3
#define OTF_VWORM_PAGESIZE      FLASH_PAGE_SIZE
#define OTF_VWORM_WORD_BYTES    FLASH_WORD_BYTES
#define OTF_VWORM_WORD_BITS     FLASH_WORD_BITS
#define OTF_VWORM_SIZE          (OTF_VWORM_PAGES * OTF_VWORM_PAGESIZE)
#define OTF_VWORM_START_PAGE    ((FLASH_FS_ADDR-FLASH_START_ADDR)/FLASH_PAGE_SIZE)
#define OTF_VWORM_START_ADDR    FLASH_FS_ADDR

#define OTF_CRC_TABLE           ENABLED //toggle
#define OTF_UHF_TABLE           DISABLED
#define OTF_UHF_TABLESIZE       0
#define OTF_M1_ENCODE_TABLE     DISABLED
#define OTF_M2_ENCODE_TABLE     ENABLED

// Total number of pages taken from program memory
#define OTF_TOTAL_PAGES         (OTF_VWORM_PAGES)




/** Abstracted Flash Organization: 
  * OpenTag uses Flash to store 2 kinds of data.  The default setup puts 
  * Filesystem memory in the back.
  * 1. Program code (obviously)
  * 2. Filesystem Memory
  *
  * FLASH_xxx constants are defined in the platform_config_xxx.h file.  
  */
#define OTF_TOTAL_SIZE          FLASH_FS_ALLOC
#define OTF_START_PAGE          OTF_VWORM_START_PAGE
#define OTF_START_ADDR          FLASH_FS_ADDR

#define OTF_VWORM_LAST_PAGE     (OTF_VWORM_START_PAGE + OTF_VWORM_PAGES - 1)
#define OTF_VWORM_END_ADDR      (FLASH_FS_ADDR + FLASH_FS_ALLOC - 1)







#endif
