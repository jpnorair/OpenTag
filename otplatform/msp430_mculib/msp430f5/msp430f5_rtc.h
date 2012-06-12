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
/** @file       /OTplatform/~MCU_MSP430/MSP430F5/msp430f5_rtc.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 Dec 2009
  * @brief      Library resources for Real Time Clock
  * @ingroup    MSP430F5 Library
  *
  ******************************************************************************
  */


/** @note
  * Not really implemented yet (too lazy), but OpenTag will require the 32 bit
  * counter mode, and not calendar mode
  */

#ifndef __MSP430F5_LIB_RTC_H
#define __MSP430F5_LIB_RTC_H

#include "msp430f5_map.h"

/*
typedef struct {
    vu16 CTL01;         // settings
    vu16 CTL23;         // settings
    vu16 PS0CTL;        // settings
    vu16 Reserved04;    // 
    vu16 Reserved06;
    vu16 PS1CTL;        // settings
    vu16 PS;            // data
    vu16 IV;            // settings
    vu16 TIM0;          // CNT12, SEC+MIN, 
    vu16 TIM1;          // CNT34, HOUR+DOWRTCNT4
    vu16 DATE;          // DATE, DAY+MON
    vu16 YEAR;          // YEARH+YEARL
    vu16 AMINHR;        // AMINHR_L+H
    vu16 ADOWDAY;       // DOWDAY_L+H
} RTC_TypeDef;
*/

//CTL01
#define RTC_IT_TEV      0x0040
#define RTC_IT_A        0x0020
#define RTC_IT_RDY      0x0010
#define RTC_IFG_TEV     0x0004
#define RTC_IFG_A       0x0002
#define RTC_IFG_RDY     0x0001

#define RTC_CTL01_BCDEnable         0x8000
#define RTC_CTL01_Hold              0x4000
#define RTC_CTL01_Mode              0x2000
#define RTC_CTL01_Mode_32bit        0x0000
#define RTC_CTL01_Mode_Calendar     0x2000

#define RTC_FLG_Ready               0x1000

#define RTC_CTL01_SSEL              0x0C00
#define RTC_CTL01_SSEL_ACLK         0x0000
#define RTC_CTL01_SSEL_SMCLK        0x0400
#define RTC_CTL01_SSEL_RT1PS        0x0800
#define RTC_CTL01_TEV               0x0300
#define RTC_CTL01_TEV_8bit          0x0000
#define RTC_CTL01_TEV_16bit         0x0100
#define RTC_CTL01_TEV_24bit         0x0200
#define RTC_CTL01_TEV_32bit         0x0300



#endif

