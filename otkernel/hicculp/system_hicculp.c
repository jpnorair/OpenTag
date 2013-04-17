/* Copyright 2010-2013 JP Norair
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
  * @file       /otkernel/hicculp/system_hicculp.c
  * @author     JP Norair
  * @version    R103
  * @date       25 Jan 2013
  * @brief      OpenTag HICCULP kernel
  * @ingroup    System
  *
  * 
  ******************************************************************************
  */

#include "build_config.h"
#if defined(__KERNEL_HICCULP__)

#include "OT_platform.h"
#include "OT_types.h"
#include "OT_utils.h"

#include "system.h"
//#include "hicculp/system_hicculp.h"   //included by system.h

#include "m2_dll.h"
#include "radio.h"
#include "session.h"
#include "mpipe.h"
#include "external.h"




/** Persistent Data Structures 
  */
sys_struct  sys;

typedef void (*fnvv)(void);




#if (OT_FEATURE(SYSTASK_CALLBACKS) == ENABLED)
#   define TASK_HANDLE(INDEX)           &sys.task[INDEX]
#   define TASK_INDEX(HANDLE)           ((HANDLE - &sys.task[0]) / sizeof(task_marker) )
#   define TASK_DECREMENT(TASK, INDEX)  TASK--
#   define TASK_INCREMENT(TASK, INDEX)  TASK++
#   define TASK_SELECT(TASK, INDEX)     TASK
#   define TASK(SELECT)                 SELECT
#   define TASK_CALL(SELECT)            SELECT->call(SELECT)
#   define TASK_INDEXED_CALL(INDEX)     sys.task[INDEX].call( &sys.task[INDEX] )
#   define TASK_MAX                     &sys.task[0]
#   define TASK_IS_IDLE(SELECT)         (SELECT == NULL)
#else
#   define TASK_HANDLE(INDEX)           INDEX
#   define TASK_INDEX(HANDLE)           HANDLE
#   define TASK_DECREMENT(TASK, INDEX)  do { TASK--; INDEX--; } while(0)
#   define TASK_INCREMENT(TASK, INDEX)  do { TASK++; INDEX++; } while(0)
#   define TASK_SELECT(TASK, INDEX)     INDEX
#   define TASK(SELECT)                 (&sys.task[SELECT])
#   define TASK_CALL(SELECT)            systask_call[SELECT]( TASK(SELECT) )
#   define TASK_INDEXED_CALL(INDEX)     systask_call[INDEX]( TASK(INDEX) )
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
#if (M2_FEATURE(ENDPOINT))
    &dll_systask_sleepscan,
#endif
#if (M2_FEATURE(BEACONS))
    &dll_systask_beacon,
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
    platform_memset((ot_u8*)sys.task, 0, sizeof(task_marker)*SYS_TASKS);
    
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
}
#endif



#ifndef EXTF_sys_panic
void sys_panic(ot_u8 err_code) {
/// Go to OFF state
    dll.idle_state = 0;
    session_flush();
    dll_idle();
    platform_disable_ktim();

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
    code    = 3; //(platform_next_ktim() <= 3) ? 0 : 3;
#   if (1)
    code   -= (sys.task_RFA.event != 0);
#   endif
#   if (OT_FEATURE(MPIPE))
    code    = (mpipe_status() >= 0) ? 1 : code;
#   endif

#   if defined(EXTF_sys_sig_powerdown)
        sys_sig_powerdown(code);
#   elif (OT_FEATURE(SYSKERN_CALLBACKS))
        sys.powerdown(code);
#   else
#       error "powerdown applet (sys_sig_powerdown) is not available."
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
    platform_disable_ktim();
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
/// Kill the indexed task.
    ot_u8 task_event;
    
    /// Always run the task exit hook, even if the task is not active.  If the
    /// task has spawned run-away interrupts or possibly other tasks, this 
    /// should stop them.  Setting event to 0 and calling will invoke the task
    /// exit hook (destructor).  This is a requirement of task implementation.
    task_event          = sys.task[i].event;
    sys.task[i].event   = 0;
    TASK_INDEXED_CALL(i);
    
    /// Check if the task was actually running.  Don't go any further if the 
    /// task had already exited.
    if (task_event != 0) {
#   if (OT_PARAM_SYSTHREADS != 0)
        /// For threaded tasks, reset the stack pointer to empty.
        ot_int offset;
        offset = (ot_int)i - TASK_Thread0;
        if (offset >= 0) {
            offset             *= OT_PARAM_SSTACK_ALLOC;
            sys.task[i].stack   = (void*)((ot_u8*)platform_ext.tstack + offset);
        }
        
        ///@todo think about changing the thread event number to negative and
        ///      nextevent to 0, which will cause it to get restarted.  Maybe
        ///      do this in its own function?
#   endif        
        
        /// Drop the context back to a stable point in the main context.  Next 
        /// time the main context is enabled (typically after all interrupts
        /// are done being serviced), the scheduler will start fresh.
        platform_drop_context(i);
    }
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
    
    do {
        sys_kill(--i);
    } while (i != 0);

    
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

void sys_task_setnext(ot_task task, ot_u16 nextevent_ti) {
    sys_task_setnext_clocks(task, TI2CLK(nextevent_ti));
}

void sys_task_setnext_clocks(ot_task task, ot_long nextevent_clocks) {
    task->nextevent = nextevent_clocks + (ot_long)platform_get_ktim();
}



#ifndef EXTF_sys_task_disable
void sys_task_disable(ot_u8 task_id) {
#if (OT_FEATURE(EXT_TASK))
//#   ifdef __DEBUG__
//    if (task_id <= (TASK_hold - TASK_external - 1)) 
//#   endif
//    {
//        sys.task[task_id].event  = 0;
//    }
    sys.task[TASK_external].event    = 0;

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
    ///    run of this function.  Then, we start the chronometer to measure the
    ///    amount of time the scheduler actually is taking.
    elapsed = platform_get_ktim();
    platform_flush_ktim();


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
        
    nextevent   = OT_GPTIM_LIMIT;
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
#               if (OT_PARAM_SYSTHREADS != 0)
                nextnext  = nextevent;
#               endif
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
    
    /// 4. The event manager is done here.  platform_schedule_ktim() will
    ///    make sure that the task hasn't been pended during the scheduler
    ///    runtime.
#   if (OT_PARAM_SYSTHREADS == 0)
    return platform_schedule_ktim( nextevent, platform_get_ktim() );
    
#   else
    ///@todo this isn't ready yet... experimental code
    {   ot_bool test;
        ot_u16 interval;
        ot_u16 retval;
        
        interval    = platform_get_ktim();
        nextevent  -= interval;
        nextnext   -= interval;
        if (nextevent > 0) {
            interval    = (ot_u16)nextevent;
            retval      = interval;
        }
        else {
            interval = ((ot_u16)nextnext >= (ot_16)TASK(select)->reserve) ? \
                        (ot_u16)nextnext : (ot_16)TASK(select)->reserve;
            retval = 0;
            
        }
        platform_set_ktim(interval);
        return retval;
    }
#   endif
}
#endif





#ifndef EXTF_sys_task_manager
void sys_task_manager() {
/// Perform a context switch onto the active task (sys.active).  In purely
/// co-operative systems, all tasks run in the same context, so do nothing.

    // Threaded mode
#   if (OT_PARAM_SYSTHREADS != 0)
    if (sys.active >= TASK_HANDLE[TASK_thread0]) {
        //do some stuff
        // - switch contexts if thread is already running
        // - if thread not running, sys_run_task will initialize it.
    }
#   endif

}
#endif



#ifndef EXTF_sys_run_task
OT_INLINE void sys_run_task() {

    // Threaded Mode: the thread has not been started, so start it.
#   if (OT_PARAM_SYSTHREADS != 0)
    if (sys.active >= TASK_HANDLE[TASK_thread0]) {
        platform_open_context(TASK(sys.active)->stack);
        goto sys_run_task_CALL;
    }
#   endif

    // Co-operative mode: disable timer because ktasks should always run to 
    // completion without interference from the scheduler.
    platform_disable_ktim();
    
    sys_run_task_CALL:
    TASK_CALL(sys.active);
}
#endif







void sys_preempt(ot_task task, ot_uint nextevent_ti) {
/// Pre-empting will "pend" the timer.  In device terms, this is implemented
/// by manually setting the timer interrupt flag.  If a task is running while
/// this function is called (typical usage), first the task will finish and then
/// the scheduler will run anyway.
    sys_task_setnext(task, nextevent_ti);
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



