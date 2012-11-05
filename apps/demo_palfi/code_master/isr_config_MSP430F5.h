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
  * Actual ISRs are implemented in: 
  * /otplatform/msp430f5/platform_isr_MSP430F5.c
  ******************************************************************************
  */

#ifndef __ISR_CONFIG_MSP430F5_H
#define __ISR_CONFIG_MSP430F5_H

//#define __ISR_RESET_ID
//#define __ISR_SYSNMI_ID
//#define __ISR_USERNMI_ID
//#define __ISR_CB_ID
//#define __ISR_T0B0_ID
//#define __ISR_T0B1_ID
//#define __ISR_WDTI_ID
//#define __ISR_USCIA0_ID
//#define __ISR_USCIB0_ID
//#define __ISR_ADC10A_ID       //Not all MSP430F55xx
//#define __ISR_ADC12A_ID       //Not all MSP430F55xx
//#define __ISR_T0A0_ID
//#define __ISR_T0A1_ID
//#define __ISR_USB_ID
//#define __ISR_DMA_ID
//#define __ISR_T1A0_ID
//#define __ISR_T1A1_ID
//#define __ISR_P1_ID
//#define __ISR_USCIA1_ID
//#define __ISR_USCIB1_ID
//#define __ISR_T2A0_ID
//#define __ISR_T2A1_ID
//#define __ISR_P2_ID
//#define __ISR_RTCA_ID



#endif 
