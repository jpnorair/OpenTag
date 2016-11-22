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
  * @file       /include/otsys/syskern.h
  * @author     JP Norair
  * @version    R101
  * @date       26 October 2012
  * @brief      Kernel System Framework
  * @defgroup   System-Kernel
  * @ingroup    System-Kernel
  *
  * The System-Kernel Module is a universal front-end for some kernel actions.
  *
  ******************************************************************************
  */


#ifndef __OTSYS_SYSKERN_H
#define __OTSYS_SYSKERN_H

#include <otplatform.h>
#include <otsys/types.h>
#include <otsys/config.h>
#include <app/build_config.h>   // Defines kernel that was selected at build-time

#if defined(__KERNEL_HICCULP__)
#   include <otsys/kernels/system_hicculp.h>

#elif defined(__KERNEL_GULP__)
#   include <otsys/kernels/system_gulp.h>

#else
//#   warning "No Kernel Defined, OK for some testbeds."
#   ifndef __KERNEL_NONE__
#   define __KERNEL_NONE__
#   endif
    //typedef void*    ot_task;
    //typedef ot_u32   ot_task_struct;

    typedef struct task_marker_struct {
        ot_u8   event;
        ot_u8   cursor;
        ot_u8   reserve;
        ot_u8   latency;
        ot_long nextevent;
#       if (OT_PARAM_SYSTHREADS != 0)
        void*   stack;
#       endif
#       if (OT_FEATURE(SYSTASK_CALLBACKS) == ENABLED)
        void    (*call)(struct task_marker_struct *marker);
#       endif
    } task_marker;

    typedef task_marker*    ot_task;
    typedef task_marker     ot_task_struct;

    typedef void (*systask_fn)(ot_task);
#endif



#ifndef OT_GPTIM_SHIFT
#   define OT_GPTIM_SHIFT 0
#endif

#if (OT_GPTIM_SHIFT != 0)
#   define CLK_UNIT         ot_long
#   define CLK2TI(CLOCKS)   (ot_u16)(CLOCKS >> OT_GPTIM_SHIFT)
#   define TI2CLK(TICKS)    ((ot_long)TICKS << OT_GPTIM_SHIFT)
#else
#   define CLK_UNIT         ot_u16
#   define CLK2TI(CLOCKS)   (CLOCKS)
#   define TI2CLK(TICKS)    (TICKS)
#endif


typedef enum {
    TASK_idle = -1,
#if (1)
    TASK_radio = 0,
#endif
#if (OT_FEATURE(MPIPE))
    TASK_mpipe,
#endif
#if (OT_PARAM(EXOTASKS) > 0)
    OT_PARAM_EXOTASK_IDS,
#endif
#if (1)
    TASK_hold,
#endif
#if (M2_FEATURE(BEACONS))
    TASK_beacon,
#endif    
#if (1)
    TASK_sleep,
#endif
//#if(OT_FEATURE(CRON))
//    TASK_otcron,
//#endif
///@todo: rearrange user tasks with external, or simply remove external.
#if (OT_PARAM(KERNELTASKS) > 0)
    OT_PARAM_KERNELTASK_IDS,
#elif (OT_FEATURE(EXT_TASK))
    TASK_external,
#endif
    // More user processing tasks would go here
    TASK_terminus
} Task_Index;


typedef enum {
    HALT_off = 0,   // force to device off
    HALT_standby,   // force to device standby
    HALT_nopower,   // suggest that primary power supply is disconnected
    HALT_lowpower,  // suggest that primary power supply is low
    HALT_terminus
} Halt_Request;





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

typedef ACTIVE_TASK		ot_task_handle;




typedef struct {
#if (OT_FEATURE(SYSKERN_CALLBACKS) == ENABLED)
#if !defined(EXTF_sys_sig_panic)
    ot_sig  panic;
#endif
#if !defined(EXTF_sys_sig_powerdown)
    ot_sig  powerdown;
#endif
#endif
    ot_task_handle  active;
    ot_task_struct  task[TASK_terminus];
} sys_struct;

extern sys_struct sys;





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

#define RFA_INDEX       TASK_radio
#define MPA_INDEX       (RFA_INDEX+OT_FEATURE(MPIPE))
#define task_RFA        task[RFA_INDEX]
#define task_MPA        task[MPA_INDEX]

#define HSS_INDEX       TASK_hold
#define BTS_INDEX       (HSS_INDEX+(M2_FEATURE(BEACONS) == ENABLED))
#define SSS_INDEX       (BTS_INDEX+1)
#define EXT_INDEX       (SSS_INDEX+(OT_FEATURE(EXT_TASK) == ENABLED))
#define IDLE_TASKS      ((EXT_INDEX-HSS_INDEX)+1)
#define task_idle(x)    task[IO_TASKS+x]
#define task_HSS        task[HSS_INDEX]
#define task_BTS        task[BTS_INDEX]
#define task_SSS        task[SSS_INDEX]
#define task_EXT        task[EXT_INDEX]

#define SYS_TASKS       TASK_terminus










/** @brief Initializes data objects for System Module and all connected Modules
  * @param none
  * @retval none
  * @ingroup System
  * @sa platform_OT_init(), sys_refresh()
  *
  * Call this once at startup, or whenever you want to do something akin to a
  * soft restart.  It puts all non-platform modules into their default states,
  * therefore it doesn't do anything to the radio driver, veelite, MPipe, or 
  * other forms of platform I/O.  For a full restart of OpenTag, instead call 
  * platform_OT_init().  If you just want to bring OpenTag back to its default 
  * idle state, without clobbering objects & callbacks, use sys_refresh().
  * 
  * Non-Platform modules that may contain data objects are:
  * <LI> Authentication module (auth) </LI>
  * <LI> Buffers module (buffers) </LI>
  * <LI> External module (ext) </LI>
  * <LI> M2QP Transport Layer module (m2qp) </LI>
  * <LI> Network Layer module (network) </LI>
  *
  */
void sys_init();




/** @brief System Task Initialization MACRO
  * @param SELF         (ot_task) This is the handle to the task 
  * @param DESTRUCTOR   (macro) This will be evaluated to kill the task
  * @param INITIALIZER  (macro) This will be evaluated to initialize the task
  * @param SELF
  * @retval none
  * @ingroup System
  *
  * As this is a MACRO, the DESTRUCTOR and INITIALIZER parameters get evaluated
  * as supplied, allowing the freedom to pass parameters from the task context.
  *
  * Per the way OpenTag Kernel Tasks work, if task->cursor = 0, the task will
  * be killed and not restarted.  If task->cursor != 0, the task will be killed
  * and restarted.
  * 
  * This MACRO should be executed from the task.  It is the normal procedure
  * for task->event = 0.  You could alternatively write your own routine for
  * event=0, but 99% of the time this is what you should do.  System functions
  * sys_init(), sys_kill_active(), and other kill/restart functions will call
  * the task(s) using event=0 to activate this MACRO (or whatever is implemented
  * for the 0-event, in the task).
  *
  */
#define sys_taskinit_macro(SELF, DESTRUCTOR, INITIALIZER)  do { \
    DESTRUCTOR; if(SELF->cursor) { INITIALIZER; SELF->cursor=0; }} while(0)




/** @brief System Call to instantiate "Kernel Panic"
  * @param err_code     (ot_u8) error code, similar to some LINUX signals
  * @retval None
  * @ingroup System
  * @sa sys_sig_panic
  *
  * When called, sys_panic() will shut-down the OpenTag kernel and then invoke
  * the panic applet, so that the user can do whatever he wants to do in 
  * response to the panic.  Usually the best policy is to restart the device 
  * and log the error somewhere (up to the application).  There is a DASH7 M1
  * ISF that logs HW faults, and generally it should be used by sys_panic().
  *
  * Official Panic error codes:
  * <LI> 01: HW or power failure (compare SIGHUP)                       </LI>
  * <LI> 07: Physical Memory Violation (compare SIGBUS)                 </LI>
  * <LI> 10: HW Data Error, typ Flash (compare SIGUSR1)                 </LI>
  * <LI> 11: Virtual addressing Segmentation Fault (compare SIGSEGV)    </LI>
  * <LI> 12: MPipe Bus Error, typ USB (compare SIGSUSR2)                </LI>
  */
void sys_panic(ot_u8 err_code);



/** @brief System Call to enact low-power routine ("sleep")
  * @param None
  * @retval None
  * @ingroup System
  * @sa sys_sig_powerdown()
  *
  * When called, sys_powerdown() will prepare a signal code based on the state
  * of the kernel and invoke the powerdown applet, so that the user can 
  * implement an application-specific sleep routine if desired.  If system 
  * callbacks are disabled, the default sleep routine will be invoked.
  *
  * @note sys_powerdown() calls sys.powerdown(code) or sys_sig_powerdown(code)
  * if system callbacks are enabled.  The value "code" it supplies is a number
  * (0, 1, 2, 3) that is the system's recommendation of how deep to sleep.
  * 0 means sleep with fastest wakeup, 1 means a sleep compatible with a wired
  * IO process, 2 means a sleep compatible with RF, and 3 means the deepest
  * sleep where the kernel timer and RAM are preserved.
  */
void sys_powerdown();




/** @brief A System Call that tells system it might need to halt
  * @param none
  * @retval (none)  
  * @ingroup System
  * @sa platform_check_halt()
  * @sa sys_powerdown()
  * @sa sys_resume()
  *
  * Device drivers can call this function if they have reason to believe the 
  * system might need to be halted.  For example, a USB driver should call this
  * function as sys_halt(HALT_lowpower) if it gets a "Suspend" command from the
  * host, or sys_halt(HALT_nopower) if it detects a disconnection.
  *
  * sys_halt() is implemented as a wrapper to sys_sig_halt(), or sys.halt(),
  * either of which the user should implement as an applet.  If there is no
  * static signal or dynamic signal configured, sys_halt() will implement the
  * default behavior, which assumes there is no secondary power supply.
  */
void sys_halt(Halt_Request halt_request);



/** @brief A System Call to resume after halt condition
  * @param none
  * @retval (none)  
  * @ingroup System
  * @sa sys_halt()
  *
  * Device drivers can call this function if they need the system to be resumed
  * after a halt they might have signalled, previously.
  *
  * If the system is not actually halted, this function will do nothing.
  */
void sys_resume();



/** @brief  Enables and modifies an idle-time task ("user space" task).
  * @param  task_id     (ot_u8) 0-255, typically just 0
  * @param  task_ctrl   (ot_u8) A task-specific state variable: 0 disables the task.
  * @param  nextevent   (ot_long) number of ticks to sleep the task until it activates
  * @retval None
  * @ingroup System
  *
  * In typical builds, there is only one user task.  It is given task_id 0.
  * If more than one user task exist, they typically have 0<=task_id<n, where
  * 'n' is the number of user tasks.
  */
void sys_task_enable(ot_u8 task_id, ot_u8 task_ctrl, ot_u16 sleep);


/** @brief  Disables an idle-time task ("user space" task)
  * @param  task_id     (ot_u8) 0-255, typically just 0
  * @retval None
  * @ingroup System
  *
  * In typical builds, there is only one user task.  It is given task_id 0.
  * If more than one user task exist, they typically have 0<=task_id<n, where
  * 'n' is the number of user tasks.
  */
void sys_task_disable(ot_u8 task_id);



/** @brief Event Management and task clocking
  * @retval (ot_uint)   Number of ticks until next event
  * @ingroup System
  * @sa sys_preempt()
  * @sa platform_ot_run()
  * @sa platform_ot_preempt()
  *
  * sys_event_manager() only should be called by platform_ot_run() or via the
  * pre-emption process, which necessarily includes functions sys_preempt() and
  * platform_ot_preempt().
  *
  * In OpenTag, all tasks are clocked by the kernel timer.  When a task is
  * scheduled to run, it will be run.  When a task is being blocked by a higher
  * priority task, it will be blocked.  When an exotask pre-empts the kernel,
  * it may cause task changes or spawning of new tasks.  sys_event_manager()
  * handles all these things.  It is the main task scheduling algorithm and
  * custodian.
  */  
ot_uint sys_event_manager();



/** @brief Task runtime management algorithm and routine
  * @param None
  * @retval None
  * @ingroup System
  * @sa sys_run_task()
  * @sa platform_ot_run()
  *
  * sys_task_manager() should be called somewhere in the platform module and
  * nowhere else.  Typically, it is called in a timeout interrupt that gets
  * triggered the active task runs too long.  The length of the timeout should
  * be obtained from task->reserve, and activated immediately before calling
  * sys_run_task().
  *
  * If a task overruns its allocated runtime (generally it is self-allocated),
  * the kernel can do various things.  What exactly gets done is up to the 
  * kernel.  The kernel may do nothing at all, it may kill the task, it may
  * de-prioritize the task, etc.
  */
void sys_task_manager();




/** @brief Run the active task
  * @param None
  * @retval None
  * @ingroup System
  * @sa platform_ot_run()
  *
  * sys_run_task() only should be called by platform_ot_run().
  *
  * This function must be inline, in order for the task killing procedure to
  * work correctly, as it often requires stack manipulations by the platform
  * module.  All it does is wrap the task calling routine, which might be 
  * different depending on the kernel and the configuration.
  */
//OT_INLINE_H
void sys_run_task();





void sys_kill_all();


void sys_kill_active();






/** @brief  Preempt the kernel: call this from preemptive task event signals.
  * @param  task			(ot_task) Task causing the preemption
  * @param  nextevent_ti	(ot_uint) Ticks (unit of time) until task wants servicing
  * @retval None
  * @ingroup System
  */
void sys_preempt(ot_task task, ot_uint nextevent_ti);




/** Task Control Wrappers
  */
void sys_task_setevent(ot_task task, ot_u8 event);
void sys_task_setcursor(ot_task task, ot_u8 cursor);
void sys_task_setreserve(ot_task task, ot_u8 reserve);
void sys_task_setlatency(ot_task task, ot_u8 latency);
void sys_task_setnext(ot_task task, ot_u16 nextevent_ti);
void sys_task_setnext_clocks(ot_task task, ot_long nextevent_clocks);




/** @brief  Synchronize a Kernel Task to an event (typically an RTC alarm)
  * @param  task_id     (Task_Index) Kernel Task ID (kernel dependent)
  * @retval none
  * @ingroup System
  *
  * This function is typically called from the Platform Module by an RTC alarm.
  * Technically, it could be called by any other code, as well.  When called,
  * it will prepare the task (represented by Task ID) for entry and then call
  * platform_ot_preempt() so the task can run immediately afterwards.
  */
void sys_synchronize(Task_Index task_id);


/**
  * @todo this is replaced by OTcron
  */
void sys_refresh_scheduler();





/** System Static Callbacks (optional) <BR>
  * ========================================================================<BR>
  * Static callbacks are optional and implemented in the user application code.
  * To enable a static callback, define the appropriate EXTF_sys_sig... value
  * in the extf_config.h file (which should be in the /App/[App-Name]/Code 
  * folder), and implement the function somewhere in your application code.
  */  


/** @brief System Panic callback function
  * @param code         (ot_int) a kernel error code (platform dependent)
  * @retval None
  * @ingroup System
  * @sa sys_panic()
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.panic().
  */
void sys_sig_panic(ot_int code);



/** @brief System Powerdown callback function
  * @param code         (ot_int) a Sleep Recommendation, 0-3
  * @retval None
  * @ingroup System
  * @sa sys_powerdown()
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.powerdown().
  *
  */
void sys_sig_powerdown(ot_int code);



/** @brief External process callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.EXT.process().  It is
  * called by the system module / kernel when an external data process begins.
  */
void sys_sig_extprocess(void* event_data);


/** @brief RF-Active initialization callback function
  * @param pcode        (ot_int) a radio process code
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.RFA.init().  It is called by
  * the system module / kernel when an active RF process (RX or TX) is being
  * initialized.
  * 
  * The radio process code is dependent on the kernel, but the OpenTag native 
  * kernel uses the settings below:
  * <LI>2: background scan </LI>
  * <LI>3: foreground scan </LI>
  * <LI>4: CSMA (unused) </LI>
  * <LI>5: foreground tx </LI>
  * <LI>6: background tx </LI>
  */
//void sys_sig_rfainit(ot_int pcode);


/** @brief RF-Active initialization callback function
  * @param pcode        (ot_int) a radio process code
  * @param scode        (ot_int) a radio termination status code
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.RFA.init().  It is called by
  * the system module / kernel when an active RF process (RX or TX) is being
  * initialized.
  * 
  * The radio process code is dependent on the kernel, but the OpenTag native 
  * kernel uses the settings below:
  * <LI>2: background scan </LI>
  * <LI>3: foreground scan </LI>
  * <LI>4: CSMA </LI>
  * <LI>5: foreground tx </LI>
  * <LI>6: background tx </LI>
  *
  * The status code is >= 0 when the radio process terminated on account of
  * a successful transfer of a packet.
  */
//void sys_sig_rfaterminate(ot_int pcode, ot_int scode);




#endif

