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
  * @file       /apps/demo_opmode/code/platform_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       31 July 2012
  * @brief      Board & Platform Selection for Demo Opmode app
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

#if (!defined(BOARD_MLX73Proto_E) && \
    !defined(BOARD_SX1231Proto_H152) && \
    !defined(BOARD_AG430DK_GW1) && \
    !defined(BOARD_AG430DK_EP1) && \
    !defined(BOARD_EM430RF) && \
    !defined(BOARD_eZ430Chronos))
#   define BOARD_EM430RF
#endif




/// Presently supported Boards for Demo Opmode App:
/// <LI> BOARD_MLX73Proto_E (STM32F)        </LI>
/// <LI> BOARD_SX1231Proto_H152 (STM32L)    </LI>
/// <LI> BOARD_AG430DK_GW1 (CC430)          </LI>
/// <LI> BOARD_AG430DK_EP1 (CC430)          </LI>
/// <LI> BOARD_EM430RF (CC430)            	</LI>
/// <LI> BOARD_eZ430Chronos (CC430)         </LI>

#if defined(BOARD_MLX73Proto_E)
#   include "stm32f10x/board_MLX73Proto_E.h"

#elif defined(BOARD_SX1231Proto_H152)
#   include "stm32l1xx/board_SX1231Proto_H152.h"

#elif defined(BOARD_AG430DK_GW1) || defined(BOARD_AG430DK_EP1)
#   include "cc430/board_AG430DK_GW1.h"

#elif defined(BOARD_EM430RF)
#   include "cc430/board_EM430RF.h"

#elif defined(BOARD_eZ430Chronos)
#   include "cc430/board_ez430chronos.h"

#else
#   error "No support BOARD is defined"

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
