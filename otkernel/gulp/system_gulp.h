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
  */
/**
  * @file       /otlib/system_gulp.h
  * @author     JP Norair
  * @version    V1.0
  * @date       16 October 2012
  * @brief      System Framework Definition for GULP Kernel
  * @ingroup    System
  *
  * The GULP kernel means: Global [interrupt] Ultra Low Power.  It is a good
  * kernel to use for devices with Global interrupt only (e.g. MSP430).  It is
  * also lightweight and fast, emphasizing low power over high performance. 
  * That is, it has a good scheduler for optimizing latency of critical & power
  * hungry I/O tasks, but it is not the best at maximizing CPU utilization.
  * 
  * These attributes make GULP a good choice for endpoints & subcontrollers.  
  * It is OK for simple gateways, too, for example a gateway with just an MPipe
  * interface and some simple control logic.
  *
  * If you are using a device with a more sophisticated interrupt controller,
  * such as a Cortex M3, you can use the HICCULP kernel to get virtually all
  * the same benefits as GULP, but better support for intensive multi-tasking.
  * 
  ******************************************************************************
  */


#ifndef __SYSTEM_NATIVE_H
#define __SYSTEM_NATIVE_H

#include "OT_types.h"
#include "OT_config.h"



/** SYS Configuration
  * This may at some point find its way into OT_config.h
  * 
  * SYS_EVENT_MAX       max duration in ticks that an event may be queued
  * SYS_RUN_MAX         max duration in ticks that the sys_run can go between calls
  * SYS_BEACON_THRESH   block beacons that are this many ticks away from a priority comm event
  */
#define SYS_EVENT_MAX               (ot_long)2147483647
#define SYS_RUN_MAX                 65535
#define SYS_BEACON_THRESH           (M2_FEATURE(BEACON_THRESH) * PLATFORM_GPTIM_TICKS_PER_TI)



/** Event processor Functions
  * This is the function type required of event processors.  I haven't yet
  * implemented external events, but it is basically one line of code away (it
  * is commented-out).
  *
  */
typedef ot_int (*ot_sysevt)();

void sys_set_extevent(ot_u8 event_no, ot_long nextevent);
void sys_set_extprocess(ot_sigv process);



/** Tasking & Event Mechanism   <BR>
  * ========================================================================<BR>
  * The Native Kernel is an pre-emptive multitasking, reservation-based,
  * externally contexted design.  It supports pre-emptive context switching
  * via external mechanisms, namely the platform module or actual HW.  
  *
  * External context switchng:
  * For example, the MSP430 has a global interrupt system, so it would require
  * a software-based context-switcher to be implemented in platform_ot_run() 
  * and platform_ot_preempt().  You can also NOT implement them if your app 
  * does not need them, which is probably the case if you are using a 
  * lightweight chip like the MSP430.
  *
  * On something like a Cortex M3, such as STM32, there is a sophisticated
  * NVIC interruptor that can do up to 8 priorities of context switching in HW.
  * So, you would only need a software context switch if you had more than 7
  * interrupt-driven tasks.
  */
#ifndef OT_FEATURE_EXTERNAL_EVENT
#define OT_FEATURE_EXTERNAL_EVENT	ENABLED
#endif

#if (OT_FEATURE(EXTERNAL_EVENT))
#define EXTERNAL_TASKS	1
#endif

#define RFA_INDEX       0
#define MPA_INDEX       (RFA_INDEX+(OT_FEATURE(MPIPE) == ENABLED))
#define IO_TASKS        ((MPA_INDEX-RFA_INDEX)+1)
#define task_RFA        task[RFA_INDEX]
#define task_MPA        task[MPA_INDEX]

#define HSS_INDEX       IO_TASKS
#define SSS_INDEX       (HSS_INDEX+(M2_FEATURE(ENDPOINT) == ENABLED))
#define BTS_INDEX       (SSS_INDEX+(M2_FEATURE(BEACONS) == ENABLED))
#define EXT_INDEX       (BTS_INDEX+(OT_FEATURE(EXT_TASK) == ENABLED))
#define IDLE_TASKS      ((EXT_INDEX-HSS_INDEX)+1)
#define task_idle(x)    task[IO_TASKS+x]
#define task_HSS        task[HSS_INDEX]
#define task_SSS        task[SSS_INDEX]
#define task_BTS        task[BTS_INDEX]
#define task_EXT        task[EXT_INDEX]

#define SYS_TASKS       (IDLE_TASKS+IO_TASKS)




typedef enum {
    TASK_idle = -1,
#if (1)
    TASK_radio = 0,
#endif
#if (OT_FEATURE(MPIPE))
    TASK_mpipe,
#endif
#if (1)
    TASK_hold,
#endif
#if (M2_FEATURE(ENDPOINT))
    TASK_sleep,
#endif
#if (M2_FEATURE(BEACONS))
    TASK_beacon,
#endif
#if (OT_FEATURE(EXT_TASK))
    TASK_external,
#endif
    // More user processing tasks would go here
    TASK_terminus
} Task_Index;





/** Task Data   <BR>
  * ========================================================================<BR>
  * The kernel manages Tasks.  Each task is identified by 8 information bytes
  * and an optional dynamic callback.  This task structure is called a "marker"
  * because the task callback is not always stored inside -- if it were, then 
  * the most proper name would be "handle."  
  *
  * task_marker data elements:
  * <LI> event:     a state variable for the task.  0 = off, 1-255 = on</LI>
  * <LI> cursor:    another state variable for the task, independent of event. 
  *                 It is only affected by the scheduler, which will set to 0
  *                 on calls to sys_synchronize() </LI>
  * <LI> reserve:   number of ticks the task estimates it needs to run </LI>
  * <LI> latency:   number of ticks latency the task can afford.  If you set this
  *                 to 0, the active task will block all lower priority tasks. </LI>
  * <LI> nextevent: number of kernel clocks between present time (actually last 
  *                 kernel exit) and when the tasks expects to be serviced 
  *                 again by the kernel.</LI>
  * <LI> call:      [Optional] Task callback. </LI>
  *
  * @note When dynamic callbacks are disabled, the kernel will use a static 
  * callback method instead.  The call element, therefore, should never be used 
  * in code outside /app, for applications where the author is certain that 
  * dynamic callbacks are enabled.
  */
typedef struct task_marker_struct {
    ot_u8   event;
    ot_u8   cursor;
    ot_u8   reserve;
    ot_u8   latency;
    ot_long nextevent;
#   if (OT_FEATURE(SYSTASK_CALLBACKS) == ENABLED)
    void    (*call)(struct task_marker_struct *marker);
#   endif
} task_marker;

typedef task_marker* ot_task;


typedef void (*systask_fn)(task_marker*);


/** System main structure  <BR>
  * ========================================================================<BR>
  * Primarily, the System structure is used to store task data.  Additionally,
  * it will store any kernel callback signals that are enabled.  At present,
  * there are two available: panic and powerdown.
  */
#if (OT_FEATURE(SYSTASK_CALLBACKS))
#	define ACTIVE_TASK	ot_task
#else
#	define ACTIVE_TASK	Task_Index
#endif

typedef struct {
#if (OT_FEATURE(SYSKERN_CALLBACKS) == ENABLED)
#if !defined(EXTF_sys_sig_panic)
    ot_sig  panic;
#endif
#if !defined(EXTF_sys_sig_powerdown)
    ot_sig  powerdown;
#endif
#endif
    ACTIVE_TASK	active;
    task_marker task[SYS_TASKS];
} sys_struct;

extern sys_struct sys;





#endif

