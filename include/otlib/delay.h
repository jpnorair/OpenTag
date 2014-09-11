/* Copyright 2013-14 JP Norair
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
