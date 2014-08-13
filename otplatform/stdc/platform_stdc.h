/* Copyright 2013 JP Norair
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
  * @version    R101
  * @date       24 Mar 2013
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



#define OT_GPTIM_LIMIT   60000



/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * Assume x86 compiled with GCC.
  */
#define PLATFORM(VAL)           PLATFORM_##VAL

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif

// How many bytes is a pointer?
#define PLATFORM_POINTER_SIZE   sizeof(void*)

// Big-endian to Platform-endian
#define PLATFORM_ENDIAN16(VAR16)        (((VAR16 >> 8) & 0x00FF) | ((VAR16 << 8) & 0xFF00))
#define PLATFORM_ENDIAN32(VAR32)        ( ((VAR32 >> 24) & 0x000000FF) \
                                        | ((VAR32 >> 8)  & 0x0000FF00) \
                                        | ((VAR32 << 8)  & 0x00FF0000) \
                                        | ((VAR32 << 24) & 0xFF000000) )
#define PLATFORM_ENDIAN16_C(CONST16)    (ot_u16)( (((ot_u16)CONST16) << 8) | (((ot_u16)CONST16) >> 8) )





/** Emulated MCU settings     <BR>
  * ========================================================================<BR>
  * Basically, emulate an STM32L with 3 I/O ports.
  */
#define MCU_FEATURE(VAL)                MCU_FEATURE_##VAL   // FEATURE                  NOTE
#define MCU_FEATURE_SVMONITOR           DISABLED            // Auto Low V powerdown     On many MCUs
#define MCU_FEATURE_CRC                 DISABLED            // CCITT CRC16              On some MCUs
#define MCU_FEATURE_AES128              DISABLED            // AES128 engine            On some MCUs
#define MCU_FEATURE_ECC                 DISABLED            // ECC engine               Rare

#define MCU_TYPE(VAL)                   MCU_TYPE_##VAL
#define MCU_TYPE_PTRINT                 ot_s32
#define MCU_TYPE_PTRUINT                ot_u32

#define MCU_PARAM(VAL)                  MCU_PARAM_##VAL
#define MCU_PARAM_POINTERSIZE           4
#define MCU_PARAM_ERRPTR                ((ot_s32)-1)
#define MCU_PARAM_UART_9600BPS          9600
#define MCU_PARAM_UART_28800BPS         28800
#define MCU_PARAM_UART_57600BPS         57600
#define MCU_PARAM_UART_115200BPS        115200
#define MCU_PARAM_UART_250000BPS        250000
#define MCU_PARAM_UART_500000BPS        500000

#define MCU_PARAM_PORTS                 3

#define SRAM_START_ADDR         0
#define EEPROM_START_ADDR       0
#define FLASH_START_ADDR        (&platform_flash[0])
#define FLASH_START_PAGE        0
#define FLASH_PAGE_SIZE         256
#define FLASH_WORD_BYTES        2
#define FLASH_WORD_BITS         (FLASH_WORD_BYTES*8)
#define FLASH_PAGE_ADDR(VAL)    (FLASH_START_ADDR + ( (VAL) * FLASH_PAGE_SIZE) )





/** Chip Settings  <BR>
  * ========================================================================<BR>
  * @todo Check if this is even needed.  GCC is dominant compiler
  */



/** Low Power Mode Macros: (Deprecated)
  * ========================================================================<BR>
  * SLEEP_MCU():        Core off, APB on, SRAM on                       (~50 uA)
  * SLEEP_WHILE_UHF():  Core off, APB on, SRAM on                       (~10 uA)
  * STOP_MCU():         Core off, RTC on, SRAM on                       (~1.5 uA)
  * STANDBY_MCU():      Core off, clocks off, SRAM off                  (~0.2 uA)
  */
#define SLEEP_MCU()
#define SLEEP_WHILE_UHF()
#define STOP_MCU()
#define STANDBY_MCU()

#define MCU_SLEEP_WHILE_RF() SLEEP_WHILE_UHF()



/** Data section Nomenclature  <BR>
  * ========================================================================<BR>
  * @todo Check if this is even needed.  ARMGCC is dominant compiler
  */
  



/** Interrupt Nomenclature  <BR>
  * ========================================================================<BR>
  * At present, no interrupts used in emulation.
  */





/** Flash Emulation  <BR>
  * ========================================================================<BR>
  * Emulate a 4KB block for Veelite.
  */

#if (OT_FEATURE(VEELITE) == ENABLED)
  //typedef ot_u8       flash_heap[FLASH_FS_ALLOC];
    typedef ot_u8       flash_heap[4096];
    extern  flash_heap  platform_flash;
#endif





#endif
