/* Copyright 2009-2011
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
  * @file       /OTplatform/STM32H152/platform_STM32L1xx.h
  * @author     
  * @version    V1.0
  * @date       12 Feb 2012
  * @brief      Platform libraries for STM32L1xx MCUs
  * @ingroup    Platform
  *
  ******************************************************************************
  */
  

#ifndef __PLATFORM_STM32L1xx_H
#define __PLATFORM_STM32L1xx_H

#include <otsys/support.h>
#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"




/** Platform Support settings      <BR>
  * ========================================================================<BR>
  * STM32 is little endian with 4 byte pointer (32 bits), and at this stage it
  * can be compiled using GCC (RIDE, most other IDE's) or IAR's proprietary
  * compiler.
  */

#define PLATFORM(VAL)           PLATFORM_##VAL
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
#define SLEEP_MCU()         sleep_mcu()
#define STOP_MCU()          PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI)
#define STANDBY_MCU()       PWR_EnterSTANDBYMode(void)

#define MCU_SLEEP_WHILE_RF() SLEEP_WHILE_UHF()

void sleep_mcu(void);

/**********************************************************************/

#ifndef BLOCKING_UART_TX
    extern DMA_InitTypeDef  UTX_DMA_Init;
#   define USART3_TX_DMA_CHANNEL            DMA1_Channel2
#   define USART3_TX_DMA_FLAG_TC            DMA1_FLAG_TC2
#   define USART3_TX_DMA_FLAG_GL            DMA1_FLAG_GL2
#endif

#ifdef RADIO_DEBUG
    void spi_save_restore(ot_bool); // from radio driver
    // USART3 for temporary debug scaffolding
    #include <stdio.h>
    #define debug_printf     printf
#else
    #define debug_printf    // does nothing
#endif /* !RADIO_DEBUG */

#endif
