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
  * @file       /apps/demo_palfi/code_slave/isr_config_MSP430F5.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       31 October 2012
  * @brief      MSP430F5 ISR Configuration for PaLFI Master Demo
  *
  * Don't actually include this.  Include OT_platform.h instead.  This file and
  * others like it are important to the preprocessor-based configuration scheme.
  *
  * You can uncomment these if you want to negate interrupts defined by the
  * board support file (via __N_ISR...) or force-on some interrupts (via
  * __ISR...).  Some interrupts are required by OpenTag, and in certain cases
  * these will be enabled regardless of the setting in this file.
  *
  * The top-level ISRs are implemented in /otplatform/msp430f5/isr_MSP430F5.c
  ******************************************************************************
  */

#ifndef __ISR_CONFIG_MSP430F5_H
#define __ISR_CONFIG_MSP430F5_H


//#define __N_ISR_RESET   63
//#define __N_ISR_SYSNMI  62
//#define __N_ISR_USERNMI 61
//#define __N_ISR_T0B0    60
//#define __N_ISR_T0B1    59
//#define __N_ISR_CB      58
//#define __N_ISR_WDTI    57
//#define __N_ISR_USCIA0  56
//#define __N_ISR_USCIB0  55
//#define __N_ISR_ADC12A  54
//#define __N_ISR_ADC10A  54
//#define __N_ISR_T0A0    53
//#define __N_ISR_T0A1    52
//#define __N_ISR_USB     51
//#define __N_ISR_DMA     50
//#define __N_ISR_T1A0    49
//#define __N_ISR_T1A1    48
//#define __N_ISR_P1      47
//#define __N_ISR_USCIA1  46
//#define __N_ISR_USCIB1  45
//#define __N_ISR_T2A0    44
//#define __N_ISR_T2A1    43
//#define __N_ISR_P2      42
//#define __N_ISR_LCDB    41
//#define __N_ISR_RTCA    40
//#define __N_ISR_AES     39

//#define __ISR_RESET     63
//#define __ISR_SYSNMI    62
//#define __ISR_USERNMI   61
//#define __ISR_T0B0      60
//#define __ISR_T0B1      59
//#define __ISR_CB        58
//#define __ISR_WDTI      57
//#define __ISR_USCIA0    56
//#define __ISR_USCIB0    55
//#define __ISR_ADC12A    54
//#define __ISR_ADC10A    54
//#define __ISR_T0A0      53
//#define __ISR_T0A1      52
//#define __ISR_USB       51
//#define __ISR_DMA       50
//#define __ISR_T1A0      49
//#define __ISR_T1A1      48
//#define __ISR_P1        47
//#define __ISR_USCIA1    46
//#define __ISR_USCIB1    45
//#define __ISR_T2A0      44
//#define __ISR_T2A1      43
//#define __ISR_P2        42
//#define __ISR_LCDB      41
//#define __ISR_RTCA      40
//#define __ISR_AES       39

#endif 
