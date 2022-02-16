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
  * @file       /apps/mpipe_bridge/app/board_config.h
  * @author     JP Norair (jpnorair@indigresso.com)
  * @version    R100
  * @date       27 Sept 2012
  * @brief      Board & HW Selection for MPipe Bridge app
  *
  * Don't actually include this.  Include OT_platform.h instead.  This file and
  * others like it are important to the preprocessor-based configuration scheme.
  ******************************************************************************
  */

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

#include <app/build_config.h>
#include "../../_common/board_select.h"



/// Default Board setting: (Jupiter_R2). 
/// You should set BOARD_... as a constant that gets passed to the compiler as 
/// a command-line argument (or whatever is equivalent).  You can also change
/// the default if you wish.

#if !BOARD_IS_OFFICIAL()
#   warning "You are not using an official board.  You will need to alter board_config.h to include your board support headers."
#endif



/// Presently supported Boards for MPipe Bridge App:
/// @note OpenTag 2.0 requires backporting to support MSP430 platforms.
/// This is planned for a later release.  For now, use STM32L platforms only.
#if (0)

#elif defined(BOARD_IKR001)
#   include <app/isr_config_STM32L.h>
#   include <board/stm32l1xx/board_IKR001.h>

#elif defined(BOARD_Pluto)
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/nucleo_STM32L053.h>

#elif defined(BOARD_Charon_R0)
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/charon_r0.h>

#elif defined(BOARD_Nucleo_STM32L053)
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/Nucleo_STM32L053.h>

#elif (defined(BOARD_Nucleo_LRWAN1))
#   include <app/isr_config_STM32L0.h>
#   include <board/stm32l0xx/Nucleo_LRWAN1.h>

#else
#   error "Selected BOARD is not supported by this app :("

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





#endif 
