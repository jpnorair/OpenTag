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
  * @file       /OTplatform/MSP430F5/platform_MSP430F5.h
  * @author     JP Norair
  * @version    V1.0
  * @date       2 Feb 2012
  * @brief      Platform Library Macros and Functions for MSP430F5
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_MSP430F5_H
#define __PLATFORM_MSP430F5_H

#include "OT_support.h"
#include "build_config.h"
#include "msp430f5_map.h"
#include "msp430f5_lib.h"


/** Platform Support settings
  * These reference the exhaustive list of officially supported platform
  * setting options.
  */
#define PLATFORM(VAL)           PLATFORM_##VAL
#define PLATFORM_MSP430F5

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif

// How many bytes is a pointer?
///@note Filesystem contents MUST be in lower 64KB of memory space in order to
///      use 2 byte pointers.  If filesystem is in upper range, you must change
///      to 4 bytes.  Most F5's have at most 48KB in the lower space.
#define PLATFORM_POINTER_SIZE   2

// Big-endian to Platform-endian
#define PLATFORM_ENDIAN16(VAR16)    __swap_bytes(VAR16)
#define PLATFORM_ENDIAN32(VAR32)    __swap_long_bytes(VAR32)




/** Interrupt Nomenclature  <BR>
  * ========================================================================<BR>
  */
#if (CC_SUPPORT == GCC)
#   include <intrinsics.h>
#   define OT_IRQPRAGMA(VAL)        __attribute__((interrupt(VAL)))
#   define OT_INTERRUPT
#   define __even_in_range(x, y)    x
#   define __get_SR_register        __read_status_register

#elif (CC_SUPPORT == CL430)
#   define OT_IRQPRAGMA(VAL)        _Pragma(#VAL)
#   define OT_INTERRUPT             __interrupt

#elif (CC_SUPPORT == IAR_V5)

#endif



/** Data section Nomenclature  <BR>
  * ========================================================================<BR>
  */
#if (CC_SUPPORT == GCC)
#   define OT_SECTION(VAR, SECTION)     __attribute__((section(##SECTION))

#elif (CC_SUPPORT == CL430)
#   define OT_DATAPRAGMA(VAR, SECTION)  _Pragma(DATA_SECTION(##VAR, ##SECTION))

#elif (CC_SUPPORT == IAR_V5)
#endif






/** Low Power Mode Macros:
  * Within OpenTag, only SLEEP_MCU is used.  The other low power modes may
  * be used outside OpenTag, especially during idle periods in the MAC sequence.
  * STANDBY is not normally useful because it shuts off the RTC.
  *
  * SLEEP_MCU():        LPM0 - Core off, DMA on, SRAM on                (~70 uA)
  * SLEEP_WHILE_UHF():  LPM0 - Same as SLEEP_MCU()                      (~70 uA)
  * STOP_MCU():         LPM3 - Core off, ACLK on (RTC), SRAM on         (~2 uA)
  * STANDBY_MCU():      LPM5 - Core off, clocks off, SRAM off           (<0.3 uA)
  */
#define SLEEP_MCU()         PMM_EnterLPM0()
#define SLEEP_WHILE_UHF()   PMM_EnterLPM0()
#define STOP_MCU()          PMM_EnterLPM3()
#define STANDBY_MCU()       PMM_EnterLPM5()

#define MCU_SLEEP_WHILE_RF() SLEEP_WHILE_UHF()



/** #### DMA Macros
  * - How many bytes/transfers are left for the DMA
  */
#define OT_RADIODMA_RX_BYTE     (ot_u16)(OT_RADIODMA_RX->SZ)
#define OT_RADIODMA_TX_BYTE     (ot_u16)(OT_RADIODMA_TX->SZ)





#endif
