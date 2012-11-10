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
/** @file       /otplatform/msp430_mculib/cc430/cc430_conf.h
  * @author     JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      Peripheral Configuration for CC430
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_CONF_H
#define __CC430_CONF_H

#if (defined(__CCSv5__) || defined(__CCSv4__) || defined(__GCC__) || defined(__GNUC__))
#define __CC430__

#   if   defined(__CC430F5125__)
#       include "cc430f5125.h"
#       define _RTCA

#   elif defined(__CC430F5127__)
#       include "cc430f5127.h"
#       define _RTCA

#   elif defined(__CC430F5135__)
#       include "cc430f5135.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _RTCA

#   elif defined(__CC430F5137__)
#       include "cc430f5137.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _RTCA

#   elif defined(__CC430F5145__)
#       include "cc430f5145.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _RTCD

#   elif defined(__CC430F5147__)
#       include "cc430f5147.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _RTCD

#   elif defined(__CC430F6125__)
#       include "cc430f6125.h"
#       define _LCDB             // Liquid Crystal Display Bus Controller (LCD)
#       define _RTCA

#   elif defined(__CC430F6126__)
#       include "cc430f6126.h"
#       define _LCDB             // Liquid Crystal Display Bus Controller (LCD)
#       define _RTCA

#   elif defined(__CC430F6127__)
#       include "cc430f6127.h"
#       define _LCDB             // Liquid Crystal Display Bus Controller (LCD)
#       define _RTCA

#   elif defined(__CC430F6135__)
#       include "cc430f6135.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _LCDB             // Liquid Crystal Display Bus Controller (LCD)
#       define _GPIO4
#       define _RTCA

#   elif defined(__CC430F6137__)
#       include "cc430f6137.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _LCDB             // Liquid Crystal Display Bus Controller (LCD)
#       define _GPIO4
#       define _RTCA

#   elif defined(__CC430F6145__)
#       include "cc430f6145.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _LCDB             // Liquid Crystal Display Bus Controller (LCD)
#       define _GPIO4
#       define _RTCD

#   elif defined(__CC430F6147__)
#       include "cc430f6147.h"
#       define _ADC12           // 12 bit Analog to Digital Converter (ADC12)
#       define _LCDB             // Liquid Crystal Display Bus Controller (LCD)
#       define _GPIO4
#       define _RTCD

#   else
#       undef __CC430__
#   endif
#endif


#ifdef __CC430__

/** Peripherals consistent across all CC430 parts
  */
#define _SFR        // Special Function Registers (SFR)
#define _SYS        // System Configuration Registers (SYS)
#define _WDT        // Watchdog Timer(s) (WDT)
#define _WDTA
#define _UCS        // Universal Clock System (UCS)
#define _PMM        // Power Management Module (PMM)
#define _FLASH      // Flash Controller (FLASH)
#define _RAM        // RAM Controller (RAM)
#define _CRC        // CRC16 Calculation Peripheral (CRC)
#define _AES        // AES128 Calculation Peripheral (AES)
#define _RF         // CC1101 Radio (RF)
#define _REF        // Reference Module (REF)
#define _CB         // Comparator B (CB)

// Port Management (PM) and GPIO peripherals (GPIO)
#define _GPIO
#   define _PM
#   define _GPIO1
#   define _GPIO2
#   define _GPIO3
#   define _GPIO5
#   define _GPIOJ
#   define _GPIO12
#   define _GPIO34

#define _DMA        // DMA Controller and Peripherals (DMA)
#   define _DMA0
#   define _DMA1
#   define _DMA2

#define _TIMA       // Timer A peripherals
#   define _TIM0A5
#   define _TIM1A3

#define _USCI       // Universal Serial Controller Interface (USCI)
#   define _UARTA0
#   define _SPIA0
#   define _I2CA0
#   define _UARTB0
#   define _SPIB0
#   define _I2CB0



/* Device define */




#endif

#endif

