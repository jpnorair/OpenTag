/* Copyright 2014 JP Norair
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
  * @file       /apps/mpipe_bridge/app/build_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       27 Sept 2014
  * @brief      Most basic list of constants needed to configure build
  *
  * Do not include this file.  Include OTAPI.h (or OT_config.h + OT_types.h)
  * for device-independent stuff, and OT_platform.h for device-dependent stuff.
  ******************************************************************************
  */

#ifndef __BUILD_CONFIG_H
#define __BUILD_CONFIG_H

#include <otsys/support.h>
#include "../../_common/board_select.h"


/** Endian Configuration  <BR>
  * ========================================================================<BR>
  * OpenTag might be compiled on Big or Little Endian Platforms.  Endianness
  * will impact many aspects of the compilation.  Sometimes, the endianness is
  * defined in system headers or via the compiler.
  */
#if (!defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__))
#   define __LITTLE_ENDIAN__
//#   define __BIG_ENDIAN__
#endif



/** Debugging Configuration  <BR>
  * ========================================================================<BR>
  * Comment-out if you don't want the debug build additions, or if you are
  * defining __DEBUG__ as a built-in via the compiler (preferred)
  */
//#ifndef __DEBUG__
//#   define __DEBUG__
//#endif



/** Flash Boundary Configuration  <BR>
  * ========================================================================<BR>
  * You can potentially use FLASH_BOUNDARY to keep all data that goes to the 
  * MCU within the lower X bytes of the Flash memory.  In certain cases, this
  * can allow you to use free/lite versions of a compiler, or simply to keep
  * the resources within a bounded limit.  Your linker script must correspond.
  */
#ifndef FLASH_BOUNDARY
#   define FLASH_BOUNDARY   65536
#endif



/** Kernel Configuration  <BR>
  * ========================================================================<BR>
  * You can override this by including KERNEL_XXX in your compiler defines.
  * MSP430 boards default to GULP, Cortex-M boards default to HICCULP
  */
#if (!defined(KERNEL_GULP) && !defined(KERNEL_HICCULP))
#   if (!BOARD_SUPPORTS_HICCULP())
#       define KERNEL_GULP
#       define __KERNEL_GULP__
#   else
#       define KERNEL_HICCULP
#       define __KERNEL_HICCULP__

#   endif
#endif



#define OS_FEATURE(VAL)                 DISABLED                // NO OS Featuresetting just yet
#define OS_FEATURE_MEMCPY               DISABLED                //  
#define OS_FEATURE_MALLOC               DISABLED



#endif 
