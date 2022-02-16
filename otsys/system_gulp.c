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
  * @file       /otsys/system_gulp.c
  * @author     JP Norair
  * @version    R104
  * @date       27 Aug 2014
  * @brief      OpenTag GULP kernel
  * @ingroup    System-Kernel
  *
  * @todo 1.0 features need to be backported to GULP
  *
  ******************************************************************************
  */

#include <app/build_config.h>
#if defined(__KERNEL_GULP__)

#include <otstd.h>
#include <otplatform.h>

#include <otsys/syskern.h>
//#include "system_gulp.h"
#include <otsys/mpipe.h>
#include <otsys/sysext.h>

#include <m2/dll.h>
#include <m2/radio.h>
#include <m2/session.h>





/** Persistent Data Structures
  */
sys_struct  sys;

typedef void (*fnvv)(void);




#if (OT_FEATURE(SYSTASK_CALLBACKS) == ENABLED)
#   define TASK_HANDLE(INDEX)           &sys.task[INDEX]
#   define TASK_INDEX(HANDLE)           ((HANDLE - &sys.task[0]) >> 3)
#   define TASK_DECREMENT(TASK, INDEX)  TASK--
#   define TASK_INCREMENT(TASK, INDEX)  TASK++
#   define TASK_SELECT(TASK, INDEX)     TASK
#   define TASK(SELECT)                 SELECT
#   define TASK_CALL(SELECT)            SELECT->call(SELECT)
#	define TASK_MAX                     &sys.task[0]
#   define TASK_IS_IDLE(SELECT)         (SELECT == NULL)
#else
#   define TASK_HANDLE(INDEX)           INDEX
#   define TASK_INDEX(HANDLE)           HANDLE
#   define TASK_DECREMENT(TASK, INDEX)  do { TASK--; INDEX--; } while(0)
#   define TASK_INCREMENT(TASK, INDEX)  do { TASK++; INDEX++; } while(0)
#   define TASK_SELECT(TASK, INDEX)     INDEX
#   define TASK(SELECT)                 (&sys.task[SELECT])
#   define TASK_CALL(SELECT)            systask_call[SELECT]( TASK(SELECT) )
#   define TASK_MAX                     0
#   define TASK_IS_IDLE(SELECT)         (SELECT < 0)
#endif




/** Administrative Subroutines
  * ============================================================================
  * Just used to make the code nice-looking or for code-reuse
  */
static const systask_fn systask_call[]   = {
#if (1)
    &dll_systask_rf,
#endif
#if (OT_FEATURE(MPIPE))
    &mpipe_systask,
#endif
#if (1)
    &dll_systask_holdscan,
#endif
#if (M2_FEATURE(BEACONS))
    &dll_systask_beacon,
#endif
#if (M2_FEATURE(ENDPOINT))
    &dll_systask_sleepscan,
#endif
#if (OT_FEATURE(EXT_TASK))
    &ext_systask,
#endif
};





/** System Core Functions
  * ============================================================================
  */

#ifndef EXTF_sys_init
void sys_init() {

    /// Set system kernel callbacks to null (if kernel callbacks enabled)
#   if (OT_FEATURE(SYSKERN_CALLBACKS) == ENABLED)
#   if !defined(EXTF_sys_sig_panic)
        sys.panic = &otutils_sig_null;
#   endif
#   endif

    /// Set default values in system tasks.  At a minimum, this is doing a
    /// memset on the task struct to 0.  If dynamic task callbacks are enabled,
    /// also set theses callbacks to the default values.
    memset((ot_u8*)sys.task, 0, sizeof(task_marker)*SYS_TASKS);

#   if (OT_FEATURE(SYSTASK_CALLBACKS) == ENABLED)
    {
        task_marker* sys_task;
        systask_fn*  default_call;
        sys_task        = &sys.task[TASK_terminus];
        default_call    = systask_call[TASK_terminus];

        while (sys_task != &sys.task[0]) {
            sys_task--;
            default_call--;
            sys_task->call  = default_call;
        }
    }
#   endif

    sys.active = TASK_MAX;

    /// Initialize External module if enabled
#   if (OT_FEATURE(EXT_TASK) == ENABLED)
        ext_init();
#   endif

    /// Initialize DLL, which also initializes the rest of the protocol stack.
    /// In some HW, the radio must be initialized before MPipe.
    dll_init();

    /// Initialize MPipe if enabled
#   if (OT_FEATURE(MPIPE) == ENABLED)
        mpipe_connect(NULL);
#   endif

    // Flush GPTIM (Kernel Timer): unnecessary, b/c should done in platform init
    //systim_flush();
}
#endif



#ifndef EXTF_sys_panic
void sys_panic(ot_u8 err_code) {
/// Go to OFF state
    dll.idle_state = 0;
    session_flush();
    dll_idle();
    systim_disable();

#   if defined(EXTF_sys_sig_panic)
        sys_sig_panic(err_code);
#   elif (OT_FEATURE(SYSKERN_CALLBACKS) == ENABLED)
        sys.panic(err_code);
#   endif
}
#endif



#ifndef EXTF_sys_powerdown
void sys_powerdown() {
/// code = 3: No active I/O Task (goto most aggressive LP regime)
/// code = 2: RF I/O Task active
/// code = 1: MPipe or other local peripheral I/O task active
/// code = 0: Use fastest-exit powerdown mode
    ot_int code;
    code    = 3; //(systim_next() <= 3) ? 0 : 3;
#   if (1)
    code   -= (sys.task_RFA.event != 0);
#   endif
#   if (OT_FEATURE(MPIPE))
    code    = (mpipe_status() <= 0) ? 1 : code;
#   endif

#   if defined(EXTF_sys_sig_powerdown)
        sys_sig_powerdown(code);
#   elif (OT_FEATURE(SYSKERN_CALLBACKS))
        sys.powerdown(code);
#   else
    {
        static const ot_sub powerdown[] = { &MCU_SLEEP,
                                            &MCU_SLEEP_WHILE_IO,
                                            &MCU_SLEEP_WHILE_RF,
                                            &MCU_STOP
                                          };
        powerdown[code]();
    }
#   endif
}
#endif



#ifndef EXTF_sys_halt
void sys_halt(Halt_Request halt_request) {
/// sys_halt() is implemented as a wrapper to sys_sig_halt(), or sys.halt(),
/// either of which the user should implement as an applet.  If there is no
/// static signal or dynamic signal configured, sys_halt() will implement the
/// default behavior, which assumes there is no secondary power supply.

#   if defined(EXTF_sys_sig_halt)
    sys_sig_halt((ot_int)halt_request);

#   elif (OT_FEATURE(SYSKERN_CALLBACKS))
    sys.halt((ot_int)halt_request);

#   else
    sys_kill_all();
    systim_disable();
    sys_powerdown();

#   endif
}
#endif


#ifndef EXTF_sys_resume
void sys_resume() {
#   if (1)
    dll_refresh();
#   endif

#   if defined(EXTF_sys_sig_resume)
    sys_sig_resume();
#   elif (OT_FEATURE(SYSKERN_CALLBACKS))
    sys.resume();
#   endif

    platform_ot_preempt();
}
#endif



void sys_kill(Task_Index i) {
/// Kill the indexed task

    /// Set event to 0 and call the task with (task->event == 0).
    /// Tasks should implement their kill functions on event == 0.
    sys.task[i].event   = 0;
#   if (OT_FEATURE(SYSTASK_CALLBACKS) == ENABLED)
    sys.task[i].call(&sys.task[i]);
#   else
    systask_call[i](&sys.task[i]);
#   endif
}



#ifndef EXTF_sys_kill_active
void sys_kill_active() {
	Task_Index i;
	i = TASK_INDEX(sys.active);

    /// There is a possibility that sys_kill_active() was called by a task or
    /// some interrupt that occured during task runtime.  So, we will use the
    /// platform_drop_context() function.  The Task Index determines which
    /// context the task is in, although this typically means nothing to a
    /// platform that is using GULP.
    sys_kill(i);
    platform_drop_context(i);
}
#endif



#ifndef EXTF_sys_kill_all
void sys_kill_all() {
    Task_Index  i = TASK_terminus;

    while (i-- != 0) {
        sys_kill(TASK_HANDLE(i));
    }

    /// See notes from sys_kill_active();
    platform_drop_context(i);

    /// There are no tasks active.  The kernel will revise is scheduling and
    /// then go to sleep for a maximal unit of time.
    platform_ot_preempt();
}
#endif








void sys_task_setevent(ot_task task, ot_u8 event) {
    task->event = event;
}

void sys_task_setcursor(ot_task task, ot_u8 cursor) {
    task->cursor = cursor;
}

void sys_task_setreserve(ot_task task, ot_u8 reserve) {
	task->reserve = reserve;
}

void sys_task_setlatency(ot_task task, ot_u8 latency) {
	task->latency = latency;
}

void sys_task_setnext(ot_task task, ot_u32 nextevent_ti) {
	sys_task_setnext_clocks(task, (ot_long)TI2CLK(nextevent_ti));
}

void sys_task_setnext_clocks(ot_task task, ot_long nextevent_clocks) {
	task->nextevent = (ot_long)systim_get() + nextevent_clocks;
}




#ifndef EXTF_sys_task_enable
void sys_task_enable(ot_u8 task_id, ot_u8 task_ctrl, ot_u16 sleep) {
#if (OT_FEATURE(EXT_TASK))
    ot_task task;
    task        = &sys.task[TASK_external+task_id];
    task->event = task_ctrl;
    sys_task_setnext(task, sleep);
	platform_ot_preempt();
#endif
}
#endif



#ifndef EXTF_sys_task_disable
void sys_task_disable(ot_u8 task_id) {
#if (OT_FEATURE(EXT_TASK))
//#   ifdef __DEBUG__
//    if (task_id <= (TASK_hold - TASK_external - 1))
//#   endif
//    {
//        sys.task[task_id].event  = 0;
//    }
    sys.task[TASK_external+task_id].event    = 0;

#endif
}
#endif




/** System Event Manager
  * ========================================================================<BR>
  * The Event Manager is multiplexed in order to work with dynamic task calls
  * or static task calls.  Some platforms work better with dynamic, and some
  * better with static.
  */
#ifndef EXTF_sys_event_manager

ot_uint sys_event_manager() {
/// This is the task-scheduling part of the kernel.
	ot_uint      elapsed;
    ot_long      nextevent;
    task_marker* task_i;
#   if (OT_FEATURE(SYSTASK_CALLBACKS) == ENABLED)
    task_marker* select;
#   else
    Task_Index  select;
    Task_Index  i;
#   endif

    /// 1. On entry, we need to know the time that has passed since the last
    ///    run of this function, and then we can flush the timer and begin a
    ///    new loop of tasking.
    elapsed = systim_get();
    systim_flush();

    /// 2. Clock all the tasks, to find out which one to do next.
    /// <LI> Run DLL clocker.  DLL module manages some irregular tasks. </LI>
    /// <LI> 60000 clocks is used as the upper limit, in order to account
    ///      for task runtime and slop.  Tasks should NEVER run longer than
    ///      255 ticks.  Longer tasks or persistent tasks must implement
    ///      pre-emption points.  (see FFT or Crypto demos for examples)</LI>
    /// <LI> Loop through tasks and find the highest priority task that
    ///      needs to be invoked.  If there are no pending tasks, save the
    ///      nearest task (nextevent), which loads into timer on exit </LI>
    /// <LI> If a pending task is selected, loop through higher priority
    ///      tasks than the one selected to find if they need to block the
    ///      invocation of the pending task. </LI>
    dll_clock(elapsed);

    nextevent   = 60000;
    task_i      = &sys.task[TASK_terminus];
    select      = TASK_MAX;
#   if (OT_FEATURE(SYSTASK_CALLBACKS) != ENABLED)
    i           = TASK_terminus;
#   endif

    // Select the highest priority task that is active and pending
    // Note that nextevent is always clocked, allowing task blocking without
    // losing synchronization.
    while (task_i != &sys.task[0]) {
        TASK_DECREMENT(task_i, i);
        task_i->nextevent -= elapsed;
        if (task_i->event != 0) {
        	// If task's nextevent is soonest, select this task to run, and
        	// also update the "nextevent" marker.  "<=" is used for priority.
        	if (task_i->nextevent <= nextevent) {
            	nextevent = task_i->nextevent;
            	select    = TASK_SELECT(task_i, i);
            }
        	// Sometime nextevent can be negative, due to a long-waiting task.
        	// higher priority tasks will always take precedent.
        	else if (task_i->nextevent <= 0) {
                select = TASK_SELECT(task_i, i);
            }
        }
    }

    // Unselect the task if there is a higher priority task blocking it
    while (task_i < TASK(select)) {
        if (task_i->event != 0) {
            // This is the reservation condition.  If the selected task's
            // estimated runtime is greater than the latency requirement or
            // time-until-pending of a higher priority task, block this
            // selected task until the conditions change.
            if ((task_i->latency < TASK(select)->reserve) || \
                (task_i->nextevent < TI2CLK(TASK(select)->reserve))) {
                nextevent   = task_i->nextevent;
                select      = TASK_SELECT(task_i, i);
                break;
            }
        }
        TASK_INCREMENT(task_i, i);
    }

    /// 3. Set the active task callback to the selected
    sys.active = select;

    /// 4. The event manager is done here, so subtract the runtime of the
    ///    event management loop from the nextevent time that was determined
    ///    in the loop.  return 0 if there is an event that should happen
    ///    immediately.
    nextevent -= systim_get();
    if (nextevent > 0) {
    	ot_u16 interval = (ot_u16)nextevent;
    	platform_set_ktim(interval);
    	return interval;
    }
    return 0;
}
#endif





#ifndef EXTF_sys_task_manager
void sys_task_manager() {
/// Not implemented in GULP... will be implemented in "Big GULP"
}
#endif



#ifndef EXTF_sys_run_task
OT_INLINE void sys_run_task() {
/// Must be inline
	TASK_CALL(sys.active);
}
#endif







void sys_preempt(ot_task task, ot_uint nextevent_ti) {
/// Pre-empting will "pend" the timer.  In device terms, this is implemented
/// by manually setting the timer interrupt flag.  If a task is running while
/// this function is called (typical usage), first the task will finish and then
/// enable the timer interrupt via sys_runtime_manager().
	sys_task_setnext(task, (ot_u32)nextevent_ti);
	platform_ot_preempt();
}




void sys_synchronize(Task_Index task_id) {
#ifdef __DEBUG__
    //if (((ot_int)task_id < IO_TASKS) || (ot_int)task_id >= (IO_TASKS+IDLE_TASKS))
	//    return;
#endif
    sys.task[task_id].cursor    = 0;
    sys_preempt(&sys.task[task_id], 0);
}


void sys_refresh_scheduler() {
#if (M2_FEATURE(RTC_SCHEDULER))
    ot_u16  sched_ctrl;
    ot_u16  offset;
    ot_int  i, j;

    platform_clear_rtc_alarms();

    sched_ctrl = dll.netconf.active;

    for (i=TASK_hold, j=0, offset=0;
        i<TASK_external;
        i++, sched_ctrl<<=1, offset+=4) {

        if (sched_ctrl & M2_SET_HOLDSCHED)
            platform_set_rtc_alarm(j++, i, offset);
    }

#endif
}



#endif // #if from top



