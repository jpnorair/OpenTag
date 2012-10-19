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
  * @file       /otlib/system.h
  * @author     JP Norair
  * @version    V1.0
  * @date       2 October 2012
  * @brief      Kernel System Framework
  * @defgroup   System (System Module)
  * @ingroup    System
  *
  * The System Module is a universal front-end for some kernel actions.  The 
  * system implementation (system.c) is usually kernel-dependent.  For the 
  * Native Kernel (otkernel/native), the system front-end implementation and 
  * the kernel itself are implemented together in otkernel/native/system.c
  *
  ******************************************************************************
  */


#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "OT_types.h"
#include "OT_config.h"
#include "gulp/system_gulp.h"


#ifndef GPTIM_SHIFT
#   define GPTIM_SHIFT 0
#endif

#if (GPTIM_SHIFT != 0)
#   define CLK_UNIT         ot_long
#   define CLK2TI(CLOCKS)   (ot_u16)(CLOCKS >> GPTIM_SHIFT)
#   define TI2CLK(TICKS)    ((ot_long)TICKS << GPTIM_SHIFT)
#else
#   define CLK_UNIT         ot_u16
#   define CLK2TI(CLOCKS)   (CLOCKS)
#   define TI2CLK(TICKS)    (TICKS)
#endif




void SYS_WATCHDOG_RUN();


/** @brief Null callback routine for the kernel app-loading feature
  * @param none
  * @retval ot_bool     sys_loadapp_null() always returns False
  * @ingroup System
  * 
  * The System Kernel can load a user "applet" during idle time.  The
  * user applet must return True if creating a new session.  The user
  * can manage the app that gets loaded however he (or she) wants.
  */
ot_bool sys_loadapp_null(void);



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



/** @brief System Panic
  * @param err_code     (ot_u8) error code, similar to POSIX death signals
  * @retval None
  * @ingroup System
  * @sa sys_sig_panic
  *
  * When called, sys_panic() will shut-down the OpenTag kernel and then invoke
  * the sys.panic system callback (if system callbacks are enabled), so that
  * the user can do whatever he wants to do in response to the panic.  Usually
  * the best policy is to shut things down, but for less serious errors (such as
  * Bus Error and Seg fault), the user could initiate some form of emergency
  * beacon that runs from static memory.
  *
  * Known (Official) Panic error codes <BR>
  * 1   Power failure (compare SIGHUP)                  <BR>
  * 4   Physical Memory Violation (compare to SIGKILL)  <BR>
  * 7   Virtual addressing Bus Error                    <BR>
  * 11  Virtual addressing Segmentation Fault           <BR>
  */
void sys_panic(ot_u8 err_code);



/** @brief System low-power routine ("sleep")
  * @param None
  * @retval None
  * @ingroup System
  * @sa sys_sig_powerdown()
  *
  * When called, sys_powerdown() will prepare a signal code based on the state
  * of the kernel and call the sys.powerdown system callback (if system
  * callbacks are enabled), so that the user can implement an application-
  * specific sleep routine if desired.  If system callbacks are disabled, the
  * default sleep routine will be invoked.
  *
  * @note sys_powerdown() calls sys.powerdown(code) or sys_sig_powerdown(code)
  * if system callbacks are enabled.  The value "code" it supplies is a number
  * (0, 1, 2, 3) that is the system's recommendation of how deep to sleep.
  * 0 means sleep with fastest wakeup, 1 means a sleep compatible with a wired
  * IO process, 2 means a sleep compatible with RF, and 3 means the deepest
  * sleep where the kernel timer and RAM are preserved.
  */
void sys_powerdown();



/** @brief Initializes sys features from settings stored in UDB elements
  * @param none
  * @retval (none)  
  * @ingroup System
  *
  * Run this when the sys module parameter RAM needs to be refreshed.
  * For example: on power up, on task activation (if running an OS), etc.
  */
void sys_init();


/** @brief Sets mutexes based on mask input
  * @param set_mask         (ot_uint) The mutex value to set
  * @retval none
  * @ingroup System
  */
void sys_set_mutex(ot_uint set_mask);


/** @brief Clears mutexes based on mask input
  * @param clear_mask       (ot_uint) a bitmask -- 0 for each bit to clear
  * @retval none
  * @ingroup System
  */
void sys_clear_mutex(ot_uint clear_mask);


/** @brief Returns the mutex value
  * @param none
  * @retval ot_int          The mutex value
  * @ingroup System
  */
ot_int sys_get_mutex();


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
  * @sa sys_task_manager()
  * 
  * This function should be called inside sys_task_manager().  You should never
  * call it anywhere else unless you have some clever ideas.
  */  
ot_uint sys_event_manager();



/** @brief Task runtime manager: Calls tasks
  * @param None
  * @retval None
  * @ingroup System
  *
  * This function should be called in your main loop.  Ideally, there should be
  * nothing else in your main loop except this function.  The kernel will
  * automatically attach a sleep/powerdown task when no tasks need servicing.
  */
void sys_task_manager();



/** @brief  Preempt the kernel: call this from preemptive task event signals.
  * @param  task			(ot_task) Task causing the preemption
  * @param  nextevent_ti	(ot_uint) Ticks (unit of time) until task wants servicing
  * @retval None
  * @ingroup System
  */
void sys_preempt(ot_task task, ot_uint nextevent_ti);




/** Task Control Wrappers
  */
void sys_task_setevent(Task_Index id, ot_u8 event);
void sys_task_setcursor(Task_Index id, ot_u8 cursor);
void sys_task_setreserve(Task_Index id, ot_u8 reserve);
void sys_task_setlatency(Task_Index id, ot_u8 latency);
void sys_task_setnext(Task_Index id, ot_u16 next);






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


/** @brief  Refresh the Scheduler Parameters from the Active Settings
  * @param  None
  * @retval None
  * @ingroup System
  */
void sys_refresh_scheduler();





/** System Static Callbacks (optional) <BR>
  * ========================================================================<BR>
  * Static callbacks are optional and implemented in the user application code.
  * To enable a static callback, define the appropriate EXTF_sys_sig... value
  * in the extf_config.h file (which should be in the /App/[App-Name]/Code 
  * folder), and implement the function somewhere in your application code.
  */  


/** @brief Loadapp callback function
  * @param none
  * @retval ot_bool     True if your app created a session for the kernel
  * @ingroup System
  * 
  * Return True if your implementation of this function creates a session that
  * you want the kernel to manage.  Otherwise, return False.
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.loadapp().  Often, for this 
  * callback, the implementation is nicer looking when using the dynamic 
  * approach, and there is minimal difference (if any) in resource usage.
  */  
ot_bool sys_sig_loadapi(void);



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
void sys_sig_rfainit(ot_int pcode);


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
void sys_sig_rfaterminate(ot_int pcode, ot_int scode);



/** @brief Beacon process prestart callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.BTS.prestart().  It is
  * called by the system module / kernel when a beacon data process begins.
  */
void sys_sig_btsprestart(void* event_data);


/** @brief Hold-scan process prestart callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.HSS.prestart().  It is
  * called by the system module / kernel when a hold-scan data process begins.
  */
void sys_sig_hssprestart(void* event_data);


/** @brief Sleep-scan process prestart callback function
  * @param event_data   (void*) pointer to kernel-dependent event datatype
  * @retval None
  * @ingroup System
  *
  * This function is the static equivalent of the dynamic callback that is
  * typically implemented in the kernel as sys.evt.SSS.prestart().  It is
  * called by the system module / kernel when a sleep-scan data process begins.
  */
void sys_sig_sssprestart(void* event_data);







#endif

