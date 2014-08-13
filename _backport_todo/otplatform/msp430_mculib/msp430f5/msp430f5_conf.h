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
  *
  */
/** @file       /otplatform/msp430_mculib/msp430f5/msp430f5_conf.h
  * @author     JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      Peripheral Configuration for MSP430F5
  * @ingroup    MSP430F5 Library
  *
  ******************************************************************************
  */


#ifndef __MSP430F5_CONF_H
#define __MSP430F5_CONF_H

#define __MSP430F5__


/* Device define */


/** Peripherals consistent across all MSP430F5 parts
  */
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
#   define _GPIOJ
#   define _GPIO12
#   define _GPIO34
#   define _GPIO56






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


// Real Time Clock (RTC)
#define _RTC

// Universal Serial Controller Interface (USCI)
#define _USCI


// Reference Module (REF)
//#define _REF


// Comparator B (CB)
//#define _CB


// Liquid Crystal Display Bus Controller (LCD)
//#define _LCD









/******************************************************************************
 *                         Peripheral declaration                             *
 ******************************************************************************/
///@todo put peripheral confs into the list

#if (defined(__CCSv5__) || defined(__CCSv4__) || defined(__GCC__) || defined(__GNUC__))
#   if  defined(__MSP430F5503__)
#       define __MSP430F55xx__
#       include "msp430f5503.h"
#       define _GPIOU
#       define _TIM0A5
#       define _TIM1A3
#       define _TIM2A3
#       define _TIM0B7
#       define _USB
#       define _UARTA0
#       define _SPIA0
#       define _UARTA1
#       define _SPIA1
#       define _SPIB0
#       define _I2CB0
#       define _SPIB1
#       define _I2CB1

#   elif  defined(__MSP430F5509__)
#       define __MSP430F55xx__
#       include "msp430f5509.h"
#       define _GPIOU
#       define _ADC10
#       define _USB
#       define _UARTA0
#       define _SPIA0
#       define _UARTA1
#       define _SPIA1
#       define _SPIB0
#       define _I2CB0
#       define _SPIB1
#       define _I2CB1
#       define _TIM0A5
#       define _TIM1A3
#       define _TIM2A3
#       define _TIM0B7

#   elif defined(__MSP430F5510__)
#       define __MSP430F55xx__
#       include "msp430f5510.h"
#       define _GPIOU
#       define _ADC10
#       define _TIM0A5
#       define _TIM1A3
#       define _TIM2A3
#       define _TIM0B7
#       define _USB
#       define _UARTA0
#       define _SPIA0
#       define _UARTA1
#       define _SPIA1
#       define _SPIB0
#       define _I2CB0
#       define _SPIB1
#       define _I2CB1

#   elif defined(__MSP430F5529__)
#       define __MSP430F55xx__
#       include "msp430f5529.h"
#       define _GPIOU
#       define _GPIO7
#       define _GPIO8
#       define _GPIO78
#       define _ADC12
#       define _TIM0A5
#       define _TIM1A3
#       define _TIM2A3
#       define _TIM0B7
#       define _USB
#       define _UARTA0
#       define _SPIA0
#       define _UARTA1
#       define _SPIA1
#       define _SPIB0
#       define _I2CB0
#       define _SPIB1
#       define _I2CB1


#   else
#       error "Chip has not been identified as a supported __MSP430F5xxx__ in msp430f5_conf.h"
#   endif

#else
#   error "Compiler is not identified as CCSv5, CCSv4, or GCC."

#endif








#endif

