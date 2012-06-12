/* Copyright 2010-2011 JP Norair
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
  * @file       /apps/.../platform_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       16 November 2011
  * @brief      Board & Platform Selection
  *
  * Don't actually include this.  Include OT_platform.h instead.
  ******************************************************************************
  */

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include "build_config.h"


//STM32F10x Boards
//#define BOARD_MLX73Proto_E

//STM32L1xx Boards
//#define BOARD_SX1231Proto_H152

//CC430 Boards
#define BOARD_AG430DK_GW1
//#define BOARD_AG430DK_EP1
//#define BOARD_EM430RF
//#define BOARD_eZ430Chronos



#if defined(BOARD_MLX73Proto_E)
#   include "STM32F10x/board_MLX73Proto_E.h"

#elif defined(BOARD_SX1231Proto_H152)
#   include "STM32L1xx/board_SX1231Proto_H152.h"

#elif defined(BOARD_AG430DK_GW1)
#   include "CC430/board_AG430DK_GW1.h"

#elif defined(BOARD_AG430DK_EP1)
#   include "CC430/board_AG430DK_EP1.h"

#elif defined(BOARD_EM430RF)
#   include "CC430/board_EM430RF.h"

#elif defined(BOARD_eZ430Chronos)
#   include "CC430/board_eZ430Chronos.h"

#else
#   error "BOARD is set to an unknown value in platform_config.h"

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
