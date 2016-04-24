/* Copyright 2009-2011 JP Norair
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
  * @file       /OTplatform/STM32F10x/platform_STM32F10x.h
  * @author     JP Norair
  * @version    V1.0
  * @date       10 Sept 2011
  * @brief      Platform libraries for STM32F10x MCUs
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __PLATFORM_STM32F10x_H
#define __PLATFORM_STM32F10x_H

#include <otsys/support.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"




/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * STM32 is little endian with 4 byte pointer (32 bits), and at this stage it
  * can be compiled using GCC (RIDE, most other IDE's) or IAR's proprietary
  * compiler.
  */

#define PLATFORM(VAL)           PLATFORM_##VAL
#define PLATFORM_STM32F10x

#ifndef __LITTLE_ENDIAN__
#   error "Endian-ness misdefined, should be __LITTLE_ENDIAN__ (check build_config.h)"
#endif
#define PLATFORM_POINTER_SIZE       4               // How many bytes is a pointer?
#define PLATFORM_ENDIAN16(VAR16)    __REV16(VAR16)  // Big-endian to Platform-endian
#define PLATFORM_ENDIAN32(VAR32)    __REV(VAR32)    // Big-endian to Platform-endian


/** Low Power Mode Macros:
  * SLEEP keeps peripherals running, but turns off CPU.  Interrupts wake-up.
  * STOP turns off everything.  Only External interrupts (pins) wake-up.
  * STANDBY turns off everything and kills SRAM.  It can retain an RTC, though.
  *
  * In order to use STOP mode during OpenTag runtime, you need to make sure that
  * nothing is going on (i.e. no I/O activity), and you need to use an RTC alarm
  * for the kernel timer (GPTIM).  I haven't developed the method yet for using
  * RTC as kernel timer, but the foundation is there.  Stay tuned for v1.1.
  *
  * Use SLEEP Mode for testing purposes, and for now.
  */
#define SLEEP_WHILE_UHF()   SLEEP_MCU()
#define SLEEP_MCU()         __WFI()
#define STOP_MCU()          PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI)
#define STANDBY_MCU()       PWR_EnterSTANDBYMode(void)

#define MCU_SLEEP_WHILE_RF() SLEEP_WHILE_UHF()



#endif
