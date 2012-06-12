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
/** @file       /OTplatforms/~MCU_MSP430/msp430f5_lib.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Main header for MSP430F5 abstraction layer
  * @ingroup    MSP430F5 Library
  *
  ******************************************************************************
  */


#ifndef __MSP430F5_LIB_H
#define __MSP430F5_LIB_H


/*
 *  This macro is for use by other macros to form a fully valid C statement.
 */
#define st(x)      do { x } while (__LINE__ == -1)


#include "msp430f5_conf.h"
#include "msp430f5_map.h"
#include "msp430.h"
#include "platform_MSP430F5.h"      //might not be necessary


#define __mov_SR_register(x) \
    asm __volatile__("mov   %0, r2" : : "r" ((unsigned short)x) )



///@todo put peripheral confs into the list
#if (defined(__CCSv5__) || defined(__CCSv4__) || defined(__GCC__) || defined(__GNUC__))
#   if   defined(__MSP430F5509__)
#       include "msp430f5509.h"
#   elif defined(__MSP430F5510__)
#       include "msp430f5510.h"
#   elif defined(__MSP430F5529__)
#       include "msp430f5529.h"
#   else
#       error "Chip has not been identified as a supported __MSP430F5xxx__"
#   endif

#else
#   error "Compiler is not identified as CCSv5, CCSv4, or GCC."

#endif


//#include "msp430f5_adc12.h"             // 12 bit Analog to Digital Converter (ADC12)
//#include "msp430f5_aes128.h"            // AES128 Calculation Peripheral (AES)
//#include "msp430f5_cb.h"                // Comparator B (CB)
#include "msp430f5_crc.h"               // CRC16 Calculation Peripheral (CRC)
#include "msp430f5_dma.h"               // DMA Controller and Peripherals (DMA)
#include "msp430f5_flash.h"             // Flash Controller (FLASH)
#include "msp430f5_gpio.h"              // GPIO peripherals (GPIO)
//#include "msp430f5_lcd.h"               // Liquid Crystal Display Bus Controller (LCD)
#include "msp430f5_pmm.h"               // Power Management Module (PMM)
//#include "msp430f5_ram.h"               // RAM Controller (RAM)
//#include "msp430f5_ref.h"               // Reference Module (REF)
#include "msp430f5_rtc.h"               // Real Time Clock (RTC)
//#include "msp430f5_sfr.h"              // Special Function Registers (SFR)
//#include "msp430f5_sys.h"              // System Configuration Registers (SYS)
#include "msp430f5_tim.h"               // Timer peripherals
#include "msp430f5_tlv.h"               //TLV
#include "msp430f5_ucs.h"               // Clock Control
#include "msp430f5_usb.h"               // USB Device
#include "msp430f5_usci.h"              // Universal Serial Controller Interface (USCI)
//#include "msp430f5_wdt.h"               // Watchdog Timer(s) (WDT)



#endif

