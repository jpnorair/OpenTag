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
  * @file       /OTplatform/CC430/platform_CC430.h
  * @author     JP Norair
  * @version    V1.0
  * @date       2 Feb 2012
  * @brief      Platform Library Macros and Functions for CC430
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_CC430_H
#define __PLATFORM_CC430_H

#include "build_config.h"
#include "OT_support.h"
#include "cc430_lib.h"



/** Platform Support settings
  * These reference the exhaustive list of officially supported platform
  * setting options.
  */
#define PLATFORM(VAL)           PLATFORM_##VAL
#define PLATFORM_CC430

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif

// How many bytes is a pointer?
#define PLATFORM_POINTER_SIZE   2

// Big-endian to Platform-endian
#define PLATFORM_ENDIAN16(VAR16)    __swap_bytes(VAR16)
#define PLATFORM_ENDIAN32(VAR32)    __swap_long_bytes(VAR32)




/** Interrupt Nomenclature  <BR>
  * ========================================================================<BR>
  */
//#if (CC_SUPPORT == GCC)
//#   include <intrinsics.h>
//#   define OT_IRQPRAGMA(VAL)            __attribute__((interrupt(VAL)))
//#   define OT_INTERRUPT
//#   define __get_SR_register            __read_status_register
//#   define __even_in_range(x, y)        x
//#   define __OT_INTERRUPT_VECTOR(VAL)   __attribute__((interrupt(VAL)))
//#   define __OT_INTERRUPT_ISR(FN)     void ##FN (void)

//#elif (CC_SUPPORT == CL430)
#   define OT_IRQPRAGMA(VAL)            _Pragma(#VAL)
#   define OT_INTERRUPT                 __interrupt
//#   define __OT_INTERRUPT_VECTOR(VAL)   _Pragma(#VAL)
//#   define __OT_INTERRUPT_ISR(FN)       __interrupt void ##FN (void)

//#elif (CC_SUPPORT == IAR_V5)

//#endif



/** Data section Nomenclature  <BR>
  * ========================================================================<BR>
  */
//#if (CC_SUPPORT == GCC)
//#   define OT_SECTION(VAR, SECTION)     __attribute__((section(##SECTION))

//#elif (CC_SUPPORT == CL430)
#   define OT_DATAPRAGMA(VAR, SECTION)  _Pragma(DATA_SECTION(##VAR, ##SECTION))

//#elif (CC_SUPPORT == IAR_V5)
//#endif





/** Low Power Mode Macros:
  * ========================================================================<BR>
  * Within OpenTag, only SLEEP_MCU is used.  The other low power modes may
  * be used outside OpenTag, especially during idle periods in the MAC sequence.
  * STANDBY is not normally useful because it shuts off the RTC.
  *
  * SLEEP_MCU():        LPM1 - Core off, SRAM on                        (~50 uA)
  * SLEEP_WHILE_UHF():  LPM2 - Core off, SMCLK off, FLL off, SRAM on    (~10 uA)
  * STOP_MCU():         LPM3 - Core off, ACLK on (RTC), SRAM on         (~2 uA)
  * STANDBY_MCU():      LPM5 - Core off, clocks off, SRAM on            (~1 uA)
  */
  
#define MCU_SLEEP               PMM_EnterLPM1
#define MCU_SLEEP_WHILE_IO      PMM_EnterLPM1
#define MCU_SLEEP_WHILE_RF      PMM_EnterLPM2
#define MCU_STOP                PMM_EnterLPM3
#define MCU_STANDBY             PMM_EnterLPM4

//legacy deprecated
#define SLEEP_MCU()             MCU_SLEEP()
#define SLEEP_WHILE_UHF()       MCU_SLEEP_WHILE_RF()
#define STOP_MCU()              MCU_STOP()
#define STANDBY_MCU()           MCU_STANDBY()





/** #### DMA Macros
  * - How many bytes/transfers are left for the DMA
  */
#define OT_RADIODMA_RX_BYTE     (ot_u16)(OT_RADIODMA_RX->SZ)
#define OT_RADIODMA_TX_BYTE     (ot_u16)(OT_RADIODMA_TX->SZ)





#endif
