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
  * @file       /apps/demo_ponglt/code/platform_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Board & Platform Selection for Demo PongLT app
  *
  * Don't actually include this.  Include OT_platform.h instead.  This file and
  * others like it are important to the preprocessor-based configuration scheme.
  ******************************************************************************
  */

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include "build_config.h"




/// Default Board setting: (EM430RF). 
/// You should set BOARD_... as a constant that gets passed to the compiler as 
/// a command-line argument (or whatever is equivalent).  You can also change
/// the default if you wish.

#if (   !defined(BOARD_OMG_CC430)       \
    &&  !defined(BOARD_EM430RF)         \
    &&  !defined(BOARD_eZ430Chronos)    \
    &&  !defined(BOARD_RF430USB_5509)   \
    &&  !defined(BOARD_IKR001)          \
    &&  !defined(BOARD_HayTag_R1)       \
    &&  !defined(BOARD_HayTag_R1T)      \
    &&  !defined(BOARD_HayTag_R1A)      \
    )
#   define BOARD_OMG_CC430
#endif




/// Presently supported Boards for Demo PongLT App:

#if defined(BOARD_OMG_CC430)
#   include "isr_config_CC430.h"
#   include "cc430/board_OMG_CC430.h"

#elif defined(BOARD_EM430RF)
#   include "isr_config_CC430.h"
#   include "cc430/board_EM430RF.h"

#elif defined(BOARD_eZ430Chronos)
#   include "isr_config_CC430.h"
#   include "cc430/board_ez430chronos.h"

#elif defined(BOARD_RF430USB_5509)
#   include "isr_config_CC430.h"
#   include "msp430f5/board_RF430USB_5509.h"

#elif defined(BOARD_IKR001)
#   include "isr_config_STM32L.h"
#   include "stm32l1xx/board_IKR001.h"

#elif (defined(BOARD_HayTag_R1) || defined(BOARD_HayTag_R1T) || defined(BOARD_HayTag_R1A))
#   include "isr_config_STM32L.h"
#   include "stm32l1xx/board_HayTag_R1.h"

#else
#   error "No supported BOARD is defined"

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








#define OS_FEATURE(VAL)                 DISABLED                // NO OS Featuresetting just yet
#define OS_FEATURE_MEMCPY               DISABLED                //  
#define OS_FEATURE_MALLOC               DISABLED



#endif 
