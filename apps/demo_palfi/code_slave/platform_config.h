/*  Copyright 2010-2011, JP Norair
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
  */
/**
  * @file       /apps/demo_palfi/code_slave/platform_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       16 April 2012
  * @brief      Board & Platform Selection
  *
  * Don't actually include this.  Include OT_platform.h instead.
  ******************************************************************************
  */

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include "build_config.h"


//Set default board if no board is defined for this app
#if !defined(BOARD_FOB_RF430F5978)
#	warn "BOARD is not set to a supported board, using default board (FOB_RF430F5978)"
#	define BOARD_FOB_RF430F5978
#endif


#if defined(BOARD_FOB_RF430F5978)
#   include "isr_config_CC430.h"
#   include "cc430/board_FOB_RF430F5978.h"

#else

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
