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
  * @file       /include/platform/tasking.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Mar 2014
  * @brief      Platform Tasking functions
  * @ingroup    Platform
  *
  * Functions for task/thread control, preemption of the scheduler, etc.
  * The kernel will need to include this header and call these functions in
  * order to perform task management.
  *
  ******************************************************************************
  */


#ifndef __PLATFORM_TASKING_H
#define __PLATFORM_TASKING_H

#include <otsys/types.h>
#include <otsys/config.h>
#include <app/build_config.h>




/** OpenTag Tasking functions <BR>
  * ========================================================================<BR>
  */

/** @brief Drop a context (task) from the primary stack
  * @param task_id		(ot_uint) Correlated to Task_Index
  * @retval None
  * @ingroup Platform
  * @sa sys_kill_active()
  * @sa sys_kill_all()
  *
  * Do not use this function unless you know exactly what you are doing.  It
  * will perform stack manipulations that could break shit in a serious way.
  *
  * The function itself must perform platform-specific stack manipulations in
  * order to flush the stack of an active task.  Normally, this will only occur
  * when a task is being killed via calls sys_kill_active() or sys_kill_all().
  * Otherwise, the task will exit cleanly.
  *
  * The argument "task_id" matters for systems with multiple contexts.  Many
  * systems have only a single context, and in these platform_drop_context()
  * will ignore task_id.  For systems with multiple contexts, the function will
  * be more sophisticated, related to the number of simultaneous contexts
  * (program stacks) allocated by the hardware (MSP430 allows 1, Cortex M3
  * allows up to 8, etc).
  *
  * When the function is called, it must have some knowledge of the context
  * states -- this is up to the platform module code.  On a single core chip,
  * the only truly important context is the active context.  The rest can be
  * flushed rather simply.  Typically, the interrupt entry hook needs to be
  * customized to meet these requirements, such that the address where the arch
  * stores the RETI PC is saved so that it might be altered by this function.
  */
void platform_drop_context(ot_uint task_id);


/** @brief The function that pauses OpenTag
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * Pauses OpenTag until you call platform_ot_run() again.  Use this to prevent
  * OpenTag from interrupting some code you have that might be important.  Be
  * careful when you use it, though!
  *
  * @note if you do not resume OpenTag within 65535 ticks of calling pause, you
  * may run into problems.
  */
void platform_ot_pause();


/** @brief The function that invokes OpenTag -- typically 100% automatic.
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * This function will get buried into some ISR loop, usually of a timer that it
  * controls.  You can also call this function anytime you want to pre-empt the
  * internal task/event manager.  For example, if you create an ad-hoc dialog,
  * you will want to call platform_ot_run() immediately after creating it.
  */
void platform_ot_run();


/** @brief Pre-emption function for OpenTag
  * @param None
  * @retval None
  * @ingroup Platform
  *
  * This function can be called by device drivers that are linked into the
  * system module.  Basically, this means the only thing that should ever use
  * this function are the "rcevt" Radio I/O events implemented in system.c.
  * Calling it outside those confines shouldn't do anything bad, but it is not
  * necessary ... and I want to emphasize "shouldn't" ...
  */
void platform_ot_preempt();



#endif
