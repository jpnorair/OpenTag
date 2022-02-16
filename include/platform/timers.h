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

/** @typedef ot_timint
  * Type that gets used for timer input/output values.
  * Historically, OT used 16 bit timers, but now it migrating to 32 bit.
  */
typedef ot_u32  ot_timint;


///@todo Here temporarily, put this in ISRs instead
void platform_rtc_isr();



/** Chronometer functions <BR>
  * ========================================================================<BR>
  * The only function here that user code should call is systim_get_clocker(),
  * which will return the amount of time in 1/32678 second units since the
  * scheduler last ran.  The rest of the functions should only be used deep
  * inside the kernel.
  */
ot_u16 systim_get_clocker();

void systim_start_clocker();
void systim_restart_clocker();
void systim_stop_clocker();




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
  * systim_chronstamp() is not a real timer, it just is an interface to
  * a real timer that can measure intervals.  It cannot provide interrupts,
  * just interval durations.
  *
  * Initialize the interval by setting timestamp parameter to NULL.  The return
  * value in this case will be the timestamp.  To get intervals after the
  * timestamp is initialized, pass-in a pointer to your timestamp.
  *
  * Usage example:
  * my_tstamp = systim_chronstamp(NULL);
  * ...
  * ... Some things happen, possibly even exiting scope
  * ...
  * my_interval = systim_chronstamp(&my_tstamp);
  */
ot_u32 systim_chronstamp(ot_u32* timestamp);





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
void systim_set_ticker(ot_uint period);
void systim_stop_ticker();





/** @brief Initializes the OpenTag general purpose timer.
  * @param tim_init    (void*) Pointer to platform-dependent initializer
  * @retval None
  * @ingroup Platform
  *
  * On most platforms, using systim_init(NULL) will apply the default setup.
  */
void systim_init(void* tim_init);









/** @brief Returns the time elapsed since the last flush or set of ktim
  * @param None
  * @retval ot_u32      Elapsed time since last refresh
  * @ingroup Platform
  */
ot_u32 systim_get();


/** @brief Reloads ktim to update calls to systim_get()
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * This function should be only called in the kernel scheduler.
  */
void systim_flush();


/** @brief Returns the amount of timer clocks until the next interrupt
  * @param None
  * @retval ot_u16      timer clocks until next interrupt
  * @ingroup Platform
  */
ot_u16 systim_next();


void systim_pend();


/** @brief Enables GPTIM interrupt
  * @param None
  * @retval None
  * @ingroup Platform
  */
void systim_enable();
void systim_enable_insertion();



void systim_disable();
void systim_disable_insertion();



/** @brief Zeros GPTIM and sets an upper limit
  * @param value        (ot_u16)Number of ticks before next kernel event
  * @retval None
  * @ingroup Platform
  */
void platform_set_ktim(ot_u16 value);
void systim_set_insertion(ot_u16 value);
void platform_set_systim3(ot_u16 value);
void platform_set_systim4(ot_u16 value);
void platform_set_systim5(ot_u16 value);

ot_u16 systim_schedule(ot_u32 nextevent, ot_u32 overhead);













#endif
