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
  * @file       /include/platform/timers.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Platform Timer functions
  * @ingroup    Platform
  *
  * Timer functions typically used by the kernel.  Some degree of cleanup is
  * necessary in order for these to be easily put into user code.
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_TIMERS_H
#define __PLATFORM_TIMERS_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <app/build_config.h>


///@todo Here temporarily, put this in ISRs instead
void platform_rtc_isr();



/** Chronometer functions <BR>
  * ========================================================================<BR>
  * The only function here that user code should call is gptim_get_chrono(), 
  * which will return the amount of time in 1/32678 second units since the 
  * scheduler last ran.  The rest of the functions should only be used deep 
  * inside the kernel.
  */
ot_u16 gptim_get_chrono();
  
void gptim_start_chrono();
void gptim_restart_chrono();
void gptim_stop_chrono();




/** Watchdog Functionality  <BR>
  * ========================================================================<BR>
  * OpenTag doesn't guarantee that any watchdog functionality exists.  In any
  * case, here is the interface if it does.
  */
void platform_init_watchdog();
void platform_run_watchdog();
void platform_reset_watchdog(ot_u16 reset);





/** Interval Measurement  <BR>
  * ========================================================================<BR>
  */
  
/** @brief Interval measurement with 1-tick resolution
  * @param timestamp    (ot_u32*) Pointer to a tick counter
  * @retval ot_u32      Number of ticks since last initialization
  * @ingroup Platform
  *
  * platform_get_interval() is not a real timer, it just is an interface to 
  * a real timer that can measure intervals.  It cannot provide interrupts,
  * just interval durations.
  * 
  * Initialize the interval by setting timestamp parameter to NULL.  The return
  * value in this case will be the timestamp.  To get intervals after the
  * timestamp is initialized, pass-in a pointer to your timestamp.
  *
  * Usage example:
  * my_tstamp = platform_get_interval(NULL);
  * ...
  * ... Some things happen, possibly even exiting scope
  * ...
  * my_interval = platform_get_interval(&my_tstamp);
  */
ot_u32 platform_get_interval(ot_u32* timestamp);





/** I-Timer functions (itimer)  <BR>
  * ========================================================================<BR>
  * The "I-Timer" is a simple, 1 tick resolution timer.  Typically it is used
  * as a kernel watchdog, so you shouldn't mess with it unless you know exactly
  * what you are doing.
  */

/** @brief Initializes a "system tick" peripheral, but doesn't begin running it.
  * @param period : system tick period
  * @retval None
  * @ingroup Platform
  *
  * Some platforms have a dedicated SysTick peripheral (the STM32).  Others
  * may not, but almost all have some form of watchdog functionality that may
  * be used for the same purpose.
  *
  * The input parameter "period" is often the milliseconds between SysTick
  * interrupts (or flagging), but in some platforms it has a different value.
  * Please check with the implementation comments / documentation to determine
  * the usage with your platform,
  */
void platform_init_itimer(ot_uint period);
void platform_stop_itimer();





/** @brief Initializes the OpenTag general purpose timer.
  * @param prescaler    (16 bit) prescaler value.
  * @retval None
  * @ingroup Platform
  *
  * The formatting of the prescaler value can differ between platforms.  Please
  * refer to the implementation comments / documentation for more specific
  * usage information for the timer_prescaler input parameter.
  */
#ifdef PLATFORM_POSIX
    void platform_init_gptim(ot_u16 prescaler, void (*timer_handler)(int));
#else
    void platform_init_gptim(ot_u16 prescaler);
#endif








/** @brief Returns the time elapsed since the last flush or set of ktim
  * @param None
  * @retval ot_u32      Elapsed time since last refresh
  * @ingroup Platform
  */
ot_u32 platform_get_ktim();


/** @brief Reloads ktim to update calls to platform_get_ktim()
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_flush_ktim();


/** @brief Returns the amount of timer clocks until the next interrupt
  * @param None
  * @retval ot_u16      timer clocks until next interrupt
  * @ingroup Platform
  */
ot_u16 platform_next_ktim();


void platform_pend_ktim();


/** @brief Enables GPTIM interrupt
  * @param None
  * @retval None
  * @ingroup Platform
  */
void platform_enable_ktim();
void platform_enable_gptim2();



void platform_disable_ktim();
void platform_disable_gptim2();



/** @brief Zeros GPTIM and sets an upper limit
  * @param value        (ot_u16)Number of ticks before next kernel event
  * @retval None
  * @ingroup Platform
  */
void platform_set_ktim(ot_u16 value);
void platform_set_gptim2(ot_u16 value);
void platform_set_gptim3(ot_u16 value);
void platform_set_gptim4(ot_u16 value);
void platform_set_gptim5(ot_u16 value);

ot_u16 platform_schedule_ktim(ot_u32 nextevent, ot_u32 overhead);













#endif
