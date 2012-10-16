/* Copyright 2010-2012 JP Norair
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
  * @file       /otkernel/~native/system.c
  * @author     JP Norair
  * @version    V1.0
  * @date       2 May 2012
  * @brief      Native OpenTag kernel
  * @ingroup    System
  *
  * The Native Kernel is optimized for DASH7 Mode 2 usage with OpenTag.  It is
  * not designed to do anything more than a WSN or IoT embedded use-case should
  * require.  If you want to integrate a more complicated system, such as an
  * Ethernet bridge, you may want to consider using something like ChibiOS 
  * instead, which has proper context-switching and dynamic task management.
  *
  * On the other hand, the Native Kernel is incredibly small, tight, and fast.
  * It is the best choice for Endpoint or Subcontroller devices.  It is also
  * very suitable as a Gateway device, as long as the gateway device is using
  * the MPipe as an API bridge.
  *
  ******************************************************************************
  */

#include "OT_types.h"
#include "OT_utils.h"
#include "OT_config.h"
#include "OT_platform.h"

#include "system.h"
#include "gulp/system_gulp.h"

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
#   define TASK_HANDLE(ID)              &sys.task[ID]
#   define TASK_DECREMENT(TASK, INDEX)  TASK--
#   define TASK_INCREMENT(TASK, INDEX)  TASK++
#   define TASK_SELECT(TASK, INDEX)     TASK
#   define TASK(SELECT)                 SELECT
#   define TASK_CALL(SELECT)            SELECT->call(SELECT)
#	define TASK_MAX                     &sys.task[0]
#   define TASK_IS_IDLE(SELECT)        (SELECT == NULL)
#else
#   define TASK_HANDLE(ID)              ID
#   define TASK_DECREMENT(TASK, INDEX)  do { TASK--; INDEX--; } while(0)
#   define TASK_INCREMENT(TASK, INDEX)  do { TASK++; INDEX++; } while(0)
#   define TASK_SELECT(TASK, INDEX)     INDEX
#   define TASK(SELECT)                 (&sys.task[SELECT])
#   define TASK_CALL(SELECT)            systask_call[SELECT]( TASK(SELECT) )
#   define TASK_MAX                     0
#   define TASK_IS_IDLE(SELECT)        (SELECT < 0)
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
    
    /// Initialize MPipe if enabled
#   if (OT_FEATURE(MPIPE) == ENABLED)
        mpipe_init(NULL);
#   endif

    /// Initialize DLL, which also initializes the rest of the protocol stack
    dll_init();

    // Flush GPTIM (Kernel Timer): unnecessary, b/c should done in platform init
    //platform_flush_gptim();
}
#endif



#ifndef EXTF_sys_panic
void sys_panic(ot_u8 err_code) {
/// Go to OFF state
    dll.idle_state = 0;
    session_flush();
    dll_idle();
    platform_disable_gptim();

#   if defined(EXTF_sys_sig_panic)
        sys_sig_panic(err_code);
#   elif (OT_FEATURE(SYSKERN_CALLBACKS) == ENABLED)
        sys.panic(err_code);
#   endif
}
#endif



#ifndef EXTF_sys_sleep
void sys_powerdown() {
    ot_int code;
    code    = 3; //(platform_next_gptim() <= 3) ? 0 : 3;
    code    = (sys.task_RFA.event) ? 2 : code;
#   if (OT_FEATURE(MPIPE))
    code    = (sys.task_MPA.event > 0) ? 1 : code;
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





void sys_task_setevent(Task_Index id, ot_u8 event) {
    sys.task[id].event = event;
}

void sys_task_setcursor(Task_Index id, ot_u8 cursor) {
    sys.task[id].cursor = cursor;
}

void sys_task_setreserve(Task_Index id, ot_u8 reserve) {
    sys.task[id].reserve = reserve;
}

void sys_task_setlatency(Task_Index id, ot_u8 latency) {
    sys.task[id].cursor = latency;
}

void sys_task_setnext(Task_Index id, ot_u16 next) {
    sys.task[id].nextevent = TI2CLK(next);
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
    ///    run of this function, and then we can flush the timer and begin a
    ///    new loop of tasking.
    elapsed = platform_get_gptim();
    platform_flush_gptim();

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
            if (task_i->nextevent <= 0) {
                select = TASK_SELECT(task_i, i);
            }
            // Update nextevent marker in any case, if it's less than before
            if (task_i->nextevent < nextevent) {
            	nextevent = task_i->nextevent;
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
    nextevent -= platform_get_gptim();
    if (nextevent > 0) {
    	ot_u16 time = (ot_u16)nextevent;
    	platform_set_gptim(time);
    	return time;
    }
    return 0;
}
#endif




void sys_task_manager() {
/// If the active task is NULL/Idle, this means the powerdown routine should
/// run, and that the timer needs to be hot before the powerdown.  Otherwise,
/// there is a task to run, and the timer should not be hot until after the
/// task is complete.

	// Enable interrupts for pre-emptive tasks during runtime of the
	// cooperative task -- co-operative tasks are parent tasks, basically
	platform_disable_gptim();
	platform_enable_interrupts();
	TASK_CALL(sys.active);

	// Disable interrupts during event management routine
	// Powerdown mode initiation MUST enable interrupts, by the way.
	// If event manager returns 0, there is a task immediately pending,
	// so no point in powering down.
	platform_disable_interrupts();
	if (sys_event_manager()) {
		sys_powerdown();
	}
}



void sys_preempt() {
/// Pre-empting will "pend" the timer.  In device terms, this is implemented
/// by manually setting the timer interrupt flag.  If a task is running while
/// this function is called (typical usage), first the task will finish and then
/// enable the timer interrupt via sys_runtime_manager().
	platform_ot_preempt();
}




void sys_synchronize(Task_Index task_id) {
#ifdef __DEBUG__
    //if ((ot_int)task_id < IDLE_TASKS)
#endif
    {
        sys.task[task_id].cursor    = 0;
        sys.task[task_id].nextevent = 0;
        sys_preempt();
    }
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







