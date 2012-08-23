/* Copyright 2009-2012 JP Norair
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
  * @file       /otplatform/stdc/platform_stdc.h
  * @author     JP Norair
  * @version    V1.0
  * @date       2 Feb 2012
  * @brief      Platform Library Macros and Functions for STD C
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_STDC_H
#define __PLATFORM_STDC_H

#include "build_config.h"
#include "OT_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



/** Platform Support settings
  * These reference the exhaustive list of officially supported platform
  * setting options.
  */
#define PLATFORM(VAL)           PLATFORM_##VAL

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif

// How many bytes is a pointer?
#define PLATFORM_POINTER_SIZE   sizeof(void*)

// Big-endian to Platform-endian
#define PLATFORM_ENDIAN16(VAR16)    (((VAR16 >> 8) & 0x00FF) | ((VAR16 << 8) & 0xFF00))
#define PLATFORM_ENDIAN32(VAR32)    (   ((VAR32 >> 24) & 0x000000FF) \
                                      | ((VAR32 >> 8)  & 0x0000FF00) \
                                      | ((VAR32 << 8)  & 0x00FF0000) \
                                      | ((VAR32 << 24) & 0xFF000000)    )



/** Interrupt Nomenclature  <BR>
  * ========================================================================<BR>
  */
#define OT_INTERRUPT




/** Data section Nomenclature  <BR>
  * ========================================================================<BR>

#if (CC_SUPPORT == GCC)
#   define OT_SECTION(VAR, SECTION)     __attribute__((section(##SECTION))

#elif (CC_SUPPORT == CL430)
#   define OT_DATAPRAGMA(VAR, SECTION)  _Pragma(DATA_SECTION(##VAR, ##SECTION))

#elif (CC_SUPPORT == IAR_V5)
#endif
  */





/** Low Power Mode Macros:
  * Within OpenTag, only SLEEP_MCU is used.  The other low power modes may
  * be used outside OpenTag, especially during idle periods in the MAC sequence.
  * STANDBY is not normally useful because it shuts off the RTC.
  *
  */
#define SLEEP_MCU()
#define SLEEP_WHILE_UHF()
#define STOP_MCU()
#define STANDBY_MCU()

#define MCU_SLEEP_WHILE_RF() SLEEP_WHILE_UHF()





typedef ot_u8 flash_heap[FLASH_FS_ALLOC]; 

extern flash_heap  platform_flash;






#endif
