/* Copyright 2010-2012 JP Norair
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
  * @file       /apps/demo_ponglt/code/isr_config_CC430.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       31 October 2012
  * @brief      CC430 ISR Configuration for Opmode Demo app
  *
  * Don't actually include this.  Include OT_platform.h instead.  This file and
  * others like it are important to the preprocessor-based configuration scheme.
  *
  * You can uncomment these if you want to negate interrupts defined by the
  * board support file (via __N_ISR...) or force-on some interrupts (via
  * __ISR...).  Some interrupts are required by OpenTag, and in certain cases
  * these will be enabled regardless of the setting in this file.
  *
  * The top-level ISRs are implemented in /otplatform/cc430/isr_CC430.c
  ******************************************************************************
  */

#ifndef __ISR_CONFIG_CC430_H
#define __ISR_CONFIG_CC430_H


//#define __N_ISR_RESET   63
//#define __N_ISR_SYSNMI  62
//#define __N_ISR_USERNMI 61
//#define __N_ISR_CB      60
//#define __N_ISR_WDTI    59
//#define __N_ISR_USCIA0  58
//#define __N_ISR_USCIB0  57
//#define __N_ISR_ADC12A  56
//#define __N_ISR_T0A0    55
//#define __N_ISR_T0A1    54
//#define __N_ISR_RF1A    53
//#define __N_ISR_DMA     52
//#define __N_ISR_T1A0    51
//#define __N_ISR_T1A1    50
//#define __N_ISR_P1      49
//#define __N_ISR_P2      48
//#define __N_ISR_LCDB    47
//#define __N_ISR_RTCA    46
//#define __N_ISR_AES     45

//#define __ISR_RESET     63
//#define __ISR_SYSNMI    62
//#define __ISR_USERNMI   61
//#define __ISR_CB        60
//#define __ISR_WDTI      59
//#define __ISR_USCIA0    58
//#define __ISR_USCIB0    57
//#define __ISR_ADC12A    56
//#define __ISR_T0A0      55
//#define __ISR_T0A1      54
//#define __ISR_RF1A      53
//#define __ISR_DMA       52
//#define __ISR_T1A0      51
//#define __ISR_T1A1      50
//#define __ISR_P1        49
//#define __ISR_P2        48
//#define __ISR_LCDB      47
//#define __ISR_RTCA      46
//#define __ISR_AES       45

#endif
