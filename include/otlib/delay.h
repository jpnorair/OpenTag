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
  * @file       /include/otlib/delay.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Library wrappers for delay functionality
  * @defgroup   Delay
  * @ingroup    Delay
  *
  * These functions must be implemented in the platform driver section 
  *
  ******************************************************************************
  */


#ifndef __OTLIB_DELAY_H
#define __OTLIB_DELAY_H

#include <otstd.h>



/** @brief Low Power Optimized Busywait function with 1 ti resolution
  * @param n        (ot_uint) Number of ticks before resuming.
  * @retval None
  * @ingroup Delay
  * @sa delay_sti()
  * @sa delay_ms()
  * @sa delay_us()
  */
void delay_ti(ot_uint n);



/** @brief Low Power Optimized Busywait function with 1 sti resolution
  * @param  sti         (ot_u16) Number of sti to block
  * @retval None
  * @ingroup Delay
  * @delay_ms()
  * @delay_us()
  * @delay_ti()
  *
  * delay_sti() is a low-power optimizing blocking loop that does
  * not engage a watchdog timer.  In platform implementations that include
  * a programmable NMI timer, delay_sti() will probably link to the
  * same code as delay_ti().  In other implementations, delay_ti()
  * might use a watchdog timer.  So, if you want to be certain that your
  * delay process is not using a watchdog, use delay_sti().
  *
  * The timing resolution is approximately 1 sti = 1/32768s.  In many
  * implementations there is a minimum runtime of this function, due to
  * some setup overhead.  For example, on an MSP430F5 core, if sti<=10
  * the block loop will take about the same time (~300us).  Check the
  * implementation for more information.
  */
void delay_sti(ot_uint sti);




/** @brief Uses busywait loop to delay the processor for number of milliseconds
  * @param n : Number of milliseconds to delay, up to 65535
  * @retval None
  * @ingroup Delay
  */
void delay_ms(ot_uint n);




/** @brief Uses busywait loop to delay the processor for number of microseconds
  * @param n : Number of microseconds to delay, up to 65535
  * @retval None
  * @ingroup Delay
  */
void delay_us(ot_uint n);





#endif
