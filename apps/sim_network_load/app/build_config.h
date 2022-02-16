/*  Copyright 2008-2022, JP Norair
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted provided that the following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright 
  *    notice, this list of conditions and the following disclaimer in the 
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
  * POSSIBILITY OF SUCH DAMAGE.
  */
/**
  * @file       /apps/demo_ponglt/code/build_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       31 July 2012
  * @brief      Most basic list of constants needed to configure build
  *
  * Do not include this file.  Include OTAPI.h (or OT_config.h + OT_types.h)
  * for device-independent stuff, and OT_platform.h for device-dependent stuff.
  ******************************************************************************
  */

#ifndef __BUILD_CONFIG_H
#define __BUILD_CONFIG_H

#include <otsys/support.h>



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
#   if (   defined(BOARD_OMG_CC430)    \
        ||  defined(BOARD_EM430RF)      \
        ||  defined(BOARD_eZ430Chronos) \
        ||  defined(BOARD_RF430USB_5509) \
        )
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




//Deprecated
#define ISR_EMBED(VAL)                  ISR_EMBED_##VAL
#define ISR_EMBED_GPTIM                 ENABLED
#define ISR_EMBED_MPIPE                 ENABLED
#define ISR_EMBED_RADIO                 ENABLED
#define ISR_EMBED_POWER                 ENABLED
#define ISR_EMBED_RNG                   ENABLED
#define ISR_EMBED_RTC                   ENABLED







#endif 
