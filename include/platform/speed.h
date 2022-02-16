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
  * @file       /include/platform/speed.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Platform Speed Control functions
  * @ingroup    Platform
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_SPEED_H
#define __PLATFORM_SPEED_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <app/build_config.h>



/** Platform Speed Control <BR>
  * ========================================================================<BR>
  */
/** @brief  Put platform CPU into most efficient runtime speed configuration
  * @param  None
  * @retval None
  * @ingroup Platform
  *
  * Standard speed is the default speed.  On some systems like MSP430, Standard,
  * Full, and Flank may be all the same without penalty.  Other systems may
  * benefit from having speed steps.  Most STM32L configurations, for example,
  * use the MSI oscillator at 4.2 MHz as the CPU clock during Standard.
  */
void platform_standard_speed(void);


/** @brief  Put platform CPU into fastest speed allowable without setup lag
  * @param  None
  * @retval None
  * @ingroup Platform
  *
  * Full speed may be entered by some tasks that need faster performance, for
  * example, packet processing.  Transition from Standard to Full should be,
  * fast, in other words it should be a matter of oscillator switching and not
  * PLL startup.  On most STM32L configurations, for example, Full Speed
  * invokes the HSI oscillator at 16 MHz.
  */
void platform_full_speed(void);
void platform_full_off(void);



/** @brief  Fuck everything, set CPU to absolute maximum speed
  * @param  None
  * @retval None
  * @ingroup Platform
  *
  * Flank speed almost always involves firing-up a PLL, which typically has an
  * appreciable lag time (e.g. 100us), and it often incurs efficiency penalties
  * in MIPS/MHz and-or uA/MHz.  On USB-enabled configurations, Flank may be the
  * default as USB typically requires a 48MHz clock.
  *
  * Flank speed is not advisable for battery-powered devices unless there is a
  * low-duty-cycle yet demanding task such as public key cryptography, and in
  * addition there is a requirement that the task must complete in a given,
  * short amount of time.  Only use flank if both requirements exist together,
  * otherwise use full or standard speed.
  */
void platform_flank_speed(void);
void platform_flank_off(void);





/** @brief  Get clock speed in Hz from one of the system clocks
  * @param  clock_index     (ot_uint) Index of a system clock
  * @retval None
  * @ingroup Platform
  *
  * The input "clock_index" is always 0 for the main CPU clock, but different
  * CPUs/MCUs have different bus clock architectures.  See two examples below:
  *
  * MSP430F5:
  * 0: MCLK
  * 1: SMCLK
  * 2: ACLK
  *
  * Cortex M:
  * 0: AHB clock
  * 1: APB2 clock
  * 2: APB1 clock
  */
ot_ulong platform_get_clockhz(ot_uint clock_index);




#endif
