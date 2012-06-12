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
  * @file       /Apps/.../build_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    V1.0
  * @date       16 November 2011
  * @brief      Most basic list of constants needed to configure build
  *
  * Do not include this file.  Include OTAPI.h (or OT_config.h + OT_types.h)
  * for device-independent stuff, and OT_platform.h for device-dependent stuff.
  ******************************************************************************
  */

#ifndef __BUILD_CONFIG_H
#define __BUILD_CONFIG_H

#include "OT_support.h"

// Select the appropriate Endian.  This is the only platform-oriented thing you
// need to do in the build config, but it is incredibly important.
//#define __BIG_ENDIAN__
#define __LITTLE_ENDIAN__


#ifndef DEBUG_ON
// Comment-out if you don't want the debug build additions
#   define DEBUG_ON
#endif

#ifndef FLASH_BOUNDARY
// Comment-out if you don't want (or need) a Flash Boundary.
// Your linker script will need to correspond.
//#   define FLASH_BOUNDARY   30720
#endif



#define ISR_EMBED(VAL)                  ISR_EMBED_##VAL
#define ISR_EMBED_GPTIM                 ENABLED
#define ISR_EMBED_MPIPE                 ENABLED
#define ISR_EMBED_RADIO                 ENABLED
#define ISR_EMBED_POWER                 ENABLED
#define ISR_EMBED_RNG                   ENABLED
#define ISR_EMBED_RTC                   ENABLED







#endif 
