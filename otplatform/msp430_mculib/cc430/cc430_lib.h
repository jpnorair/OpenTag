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
/** @file       /otplatform/msp430_mculib/cc430/cc430_lib.h
  * @author     JP Norair
  * @version    R100
  * @date       1 Nov 2012
  * @brief      Main header for CC430 abstraction layer
  * @ingroup    CC430 Library
  *
  ******************************************************************************
  */


#ifndef __CC430_LIB_H
#define __CC430_LIB_H


/*
 *  This macro is for use by other macros to form a fully valid C statement.
 */
#define st(x)      do { x } while (__LINE__ == -1)


#include "cc430_conf.h"
#include "cc430_map.h"
#include "platform_CC430.h"


//Needed for GCC
#define __mov_SR_register(x) \
    asm __volatile__("mov   %0, r2" : : "r" ((unsigned short)x) )



// Clock Control
#ifdef _UCS
#   include "cc430_ucs.h"
#endif


// Special Function Registers (SFR)
#ifdef _SFR
// module not available yet
//#   include "cc430_sfr.h"
#endif



// System Configuration Registers (SYS)
#ifdef _SYS
// module not available yet
//#   include "cc430_sys.h"
#endif


// Watchdog Timer(s) (WDT)
#ifdef _WDT
// module not available yet
//#   include "cc430_wdt.h"
#endif


// Universal Clock System (UCS)
#ifdef _UCS
#   include "cc430_ucs.h"
#endif


// Power Management Module (PMM)
#ifdef _PMM
#   include "cc430_pmm.h"
#endif


// Flash Controller (FLASH)
#ifdef _FLASH
#   include "cc430_flash.h"
#endif


// RAM Controller (RAM)
#ifdef _RAM
// module not available yet
//#   include "cc430_ram.h"
#endif



// GPIO peripherals (GPIO)
#ifdef _GPIO
#   include "cc430_gpio.h"
#endif



// DMA Controller and Peripherals (DMA)
#ifdef _DMA
#   include "cc430_dma.h"
#endif



// CRC16 Calculation Peripheral (CRC)
#ifdef _CRC
#   include "cc430_crc.h"
#endif



// AES128 Calculation Peripheral (AES)
#ifdef _AES
// module not available yet
//#   include "cc430_aes128.h"
#endif




// Timer A peripherals
#ifdef _TIMA
#   include "cc430_tim.h"
#endif




// Real Time Clock (RTC)
#if (defined(_RTCA) || defined(_RTCD))
#   include "cc430_rtc.h"
#endif



// Universal Serial Controller Interface (USCI)
#ifdef _USCI
#   include "cc430_usci.h"
#endif




// CC1101 Radio (RF)
#ifdef _RF
#   include "cc430_rf.h"
#endif




// Reference Module (REF)
#ifdef _REF
// module not available yet
//#   include "cc430_ref.h"
#endif




// Comparator B (CB)
#ifdef _CB
// module not available yet
//#   include "cc430_cb.h"
#endif



// 10 or 12 bit Analog to Digital Converter (ADC10, ADC12)
#if defined(_ADC12)
#   include "cc430_adc12.h"          // 12 bit Analog to Digital Converter (ADC12)
#elif defined(_ADC10)
#   include "cc430_adc10.h"
#endif



// Liquid Crystal Display Bus Controller (LCD)
#ifdef _LCDB
// module not available yet
//#   include "cc430_lcd.h"
#endif



#endif

