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
  * @file       /apps/demo_palfi/code_slave/isr_config_CC430.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       31 October 2012
  * @brief      CC430 ISR Configuration for PaLFI Slave Demo
  *
  * Don't actually include this.  Include OT_platform.h instead.  This file and
  * others like it are important to the preprocessor-based configuration scheme.
  *
  * You can uncomment these if you want to use one of these interrupts in your
  * app.  Note that some interrupts will be enabled and compiled because they
  * are required by OpenTag, regardless of the setting in this file.  Also note
  * that the board config file might define __ISR_XXX constants as well.
  *
  * Actual ISRs are implemented in /otplatform/cc430/platform_isr_CC430.c
  ******************************************************************************
  */

#ifndef __ISR_CONFIG_CC430_H
#define __ISR_CONFIG_CC430_H

// The PaLFI pin I/F is on P1
// The PaLFI trimming routine usually uses Timer 1 (check board cfg)
// Both __ISR_P1 and __ISR_T1A0 get defined in the board cfg file of RF430

//#define __ISR_RESET
#define __ISR_SYSNMI
#define __ISR_USERNMI
//#define __ISR_CB
//#define __ISR_WDTI
//#define __ISR_USCIA0
//#define __ISR_USCIB0
//#define __ISR_ADC12A
//#define __ISR_T0A0
//#define __ISR_T0A1
#define __ISR_RF1A
//#define __ISR_DMA
//#define __ISR_T1A0
//#define __ISR_T1A1
//#define __ISR_P1
//#define __ISR_P2
//#define __ISR_LCDB
//#define __ISR_RTCA
//#define __ISR_AES


#endif 
