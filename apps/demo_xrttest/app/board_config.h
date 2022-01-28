/* Copyright 2010-2012 JP Norair
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
  */
/**
  * @file       /apps/ht_xrttest/app/board_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       31 July 2017
  * @brief      Board & HW Selection for Demo PongLT app
  *
  * Don't actually include this.  Include <otplatform.h> instead.  This file and
  * others like it are important to the preprocessor-based configuration scheme.
  ******************************************************************************
  */

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

#include <app/build_config.h>
#include "../../_common/board_select.h"


/// Default Board setting: (Jupiter_R2). 
/// You should set BOARD_... as a constant that gets passed to the compiler as 
/// a command-line argument (or whatever is equivalent).  You can also change
/// the default if you wish.

#if !BOARD_IS_OFFICIAL()
#   warning "You are not using an official board.  You will need to alter board_config.h to include your board support headers."
#endif




/// Presently supported Boards for Demo PongLT App:
/// @note OpenTag 2.0 requires backporting to support MSP430 platforms.
/// This is planned for a later release.  For now, use STM32L platforms only.
#if (0)

//#elif defined(BOARD_OMG_CC430)
//#   include <app/isr_config_CC430.h>
//#   include <board/cc430/board_OMG_CC430.h>
//
//#elif defined(BOARD_EM430RF)
//#   include <app/isr_config_CC430.h>
//#   include <board/cc430/board_EM430RF.h>
//
//#elif defined(BOARD_eZ430Chronos)
//#   include <app/isr_config_CC430.h>
//#   include <board/cc430/board_ez430chronos.h>
//
//#elif defined(BOARD_RF430USB_5509)
//#   include <app/isr_config_CC430.h>
//#   include <board/msp430f5/board_RF430USB_5509.h>
//
#elif defined(BOARD_IKR001)
#   include <app/isr_config_STM32L.h>
#   include <board/stm32l1xx/board_IKR001.h>

#elif defined(BOARD_Jupiter_R1)
#   include <app/isr_config_STM32L.h>
#   include <board/stm32l1xx/board_Jupiter_R1.h>

#elif defined(BOARD_Jupiter_R2)
#   include <app/isr_config_STM32L.h>
#   include <board/stm32l1xx/board_Jupiter_R2.h>

#elif (defined(BOARD_HayTag_R1) || defined(BOARD_HayTag_R1T) || defined(BOARD_HayTag_R1A))
#   include <app/isr_config_STM32L.h>
#   include <board/stm32l1xx/board_HayTag_R1.h>

#elif (defined(BOARD_Nucleo_LRWAN1))
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/Nucleo_LRWAN1.h>

#elif defined(BOARD_Nucleo_L412_LoRa)
#   include <app/isr_config_STM32L4.h>
#   include <board/stm32l4xx/board_Nucleo-L412_LoRa.h>

#elif defined(BOARD_Nucleo_WL55)
#   include <app/isr_config_STM32WL55.h>
#   include <board/stm32wl55xx/nucleo_wl55.h>

#elif (defined(BOARD_Discovery_LoRa) || defined(BOARD_Discovery_LoRa_GPS))
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/discovery_LoRa.h>

#elif defined(BOARD_HayTag_LoRa_MiniGW)
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/board_HayTag_LoRa-MiniGW.h>

#elif defined(BOARD_HayTag_LoRa)
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/board_HayTag_LoRa.h>

#elif defined(BOARD_HayTag_LoRa_R2)
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/board_HayTag_LoRa_R2.h>

#elif defined(BOARD_HayTag_LoRa_R3)
#   include <app/isr_config_STM32L4.h>
#   include <board/stm32l4xx/board_HayTag_LoRa_R3.h>

#else
#   error "Selected BOARD is not supported by this app :("

#endif




/// Macro settings: ENABLED, DISABLED, NOT_AVAILABLE
#ifdef ENABLED
#   undef ENABLED
#endif
#define ENABLED  1

#ifdef DISABLED
#   undef DISABLED
#endif
#define DISABLED  0

#ifdef NOT_AVAILABLE
#   undef NOT_AVAILABLE
#endif
#define NOT_AVAILABLE   DISABLED





#endif 
