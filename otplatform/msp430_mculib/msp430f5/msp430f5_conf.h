/* Copyright 2009 JP Norair
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
/** @file       /OTplatform/~MCU_MSP430/MSP430F5/msp430f5_conf.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Peripheral Configuration for MSP430F5
  * @ingroup    MSP430F5 Library
  *
  ******************************************************************************
  */


#ifndef __MSP430F5_CONF_H
#define __MSP430F5_CONF_H



/* Device define */





/******************************************************************************
 *                         Peripheral declaration                             *
 ******************************************************************************/


// Special Function Registers (SFR)
#define _SFR


// System Configuration Registers (SYS)
#define _SYS


// Watchdog Timer(s) (WDT)
#define _WDT
#   define _WDTA


// Universal Clock System (UCS)
#define _UCS


// Power Management Module (PMM)
#define _PMM



// Flash Controller (FLASH)
#define _FLASH



// RAM Controller (RAM)
//#define _RAM



// Port Management (PM) and GPIO peripherals (GPIO)
#define _GPIO
#   define _PM
#   define _GPIO1
#   define _GPIO2
#   define _GPIO3
#   define _GPIO4
#   define _GPIO5
#   define _GPIO6
#   define _GPIO7
#   define _GPIO8
#   define _GPIOJ
#   define _GPIOU
#   define _GPIO12
#   define _GPIO34
#   define _GPIO56
#   define _GPIO78




// DMA Controller and Peripherals (DMA)
#define _DMA
#   define _DMA0
#   define _DMA1
#   define _DMA2





// CRC16 Calculation Peripheral (CRC)
#define _CRC




// AES128 Calculation Peripheral (AES)
//#define _AES





// Timer A peripherals
#define _TIMA
#   define _TIM0A5
#   define _TIM1A3
#   define _TIM2A3
#   define _TIM0B7




// Real Time Clock (RTC)
#define _RTC



// Universal Serial Controller Interface (USCI)
#define _USCI
#   define _UART0
#   define _SPI0
#   define _I2C0
#   define _UART1
#   define _SPI1
#   define _I2C1
#   define _UART2
#   define _SPI2
#   define _I2C2
#   define _UART3
#   define _SPI3
#   define _I2C3




// Reference Module (REF)
//#define _REF





// Comparator B (CB)
//#define _CB




// 12 bit Analog to Digital Converter (ADC12)
#define _ADC12




// Liquid Crystal Display Bus Controller (LCD)
//#define _LCD



// USB Device Controller
#define _USB



#endif

