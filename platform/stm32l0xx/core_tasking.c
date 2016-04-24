/* Copyright 2014 JP Norair
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
  * @file       /otplatform/stm32l1xx/core_tasking.c
  * @author     JP Norair
  * @version    R100
  * @date       27 Aug 2014
  * @brief      Main, core platform implementation for STM32L
  * @ingroup    Platform
  *
  ******************************************************************************
  */

#include <otstd.h>
#include <otplatform.h>
#include <otsys/time.h>


/** Macros of Ill Repute <BR>
  * ========================================================================<BR>
  */
#define __SET_PENDSV()      (SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk)
#define __CLR_PENDSV()      (SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk)
#define __SEND_SVC(VAL)     asm volatile(" svc  %0" : : "I"(VAL) );





/** KERNEL INTERRUPTS
  * =================
  */
void SVC_Handler(void) {
/// At present, the only protected system call is the call to the scheduler.
/// (sys_event_manager()).  Future system calls might make-use of the SVC
/// vectoring, however.
    sys_event_manager();

#if 0 //(OT_PARAM_SYSTHREADS != 0)
    ot_u8* task_lr;
    register ot_u32* stack;

    // Caller of SVC should be a task on the P-stack
    asm volatile ("   MRS  %0, PSP" : "=r"(stack) );

    // 6 word offset is where LR from caller stack is stored.
    // LR will be instruction after SVC
    task_lr = (ot_u8*)(stack + (6*4));

    // The 16bit Thumb2 instruction before the LR address is the SVC.
    // The lower byte of this instruction is the immediate code
    switch (task_lr[-2]) {

        // Call 0: this is the call to the scheduler
        case 0:
            SVC_Handler_eventmgr:
            break;


        // Call 1: Task & Thread Killer
        // Initial Task LR was placed on P-stack by NVIC HW when this interrupt
        // occurred.  By design it has a 6-word offset from PSP.  Overwrite it
        // with the saved address that sends code to RETURN_FROM_TASK.
        case 1: 
            #   if (OT_PARAM_SYSTHREADS != 0)
                // Here would go some context clearing jazz
            #   endif
            
            if (platform_ext.task_exit != NULL) {
                stack[6] = (ot_u32)platform_ext.task_exit;
            }
            break;


        // Call 2: Save a Context.  
        //         This also acts as a context switch, because it falls-through to
        //         the "load context" operation.
        case 2: {
            asm volatile (
            // 1. Offset Stack by 32 because we are going to store 8 registers on it
            "   SUB     %0, %0, #32\n" : "=r"(stack)
    
            // 2. Use Store-Multiple to do R4-R7.  It works in reverse and auto-
            //    increments Stack (%0)
            "   STM     %0!, {r4-r7}\n"
    
            // 3. Move r8-r11 into r4-r7 since store instructions cannot use r8-11 on CM0.
            "   MOV     r4, r8\n"
            "   MOV     r5, r9\n"
            "   MOV     r6, r10\n"
            "   MOV     r7, r11\n"
    
            // 4. Use Store-Multiple to do R8-R11, just like previously
            "   STM     %0!, {r4-r7}\n"
    
            // 5. Re-offset the Stack, because two calls to STM have brought it back
            //    to its starting point. (not necessary, simply used as a feature)
            //"   SUB     %0, %0, #32\n"
            );
        }
        
        
        // Call 3: Load a context.
        //         - if sys_thread_manager() returns 0, there is no thread to run.
        //         - else, it returns the dormant thread stack pointer (dtsp)
        case 3: {
            register ot_u32 dtsp;
            dtsp = (ot_u32)sys_thread_manager();
            if (dtsp == 0) break;
            
            asm volatile (
            // 1. First get the high registers via LDM, which requires subtracting 
            //    16 bytes to get to the front of that block.
            "   SUB     %0, %0, #16\n" : "=r"(dtsp)
            "   LDMIA   %0!, {r4-r7}\n"
    
            // 2. LDM cannot directly access r8-r11, so they must be paged out.
            "   MOV     r8, r4\n"
            "   MOV     r9, r5\n"
            "   MOV     r10, r6\n"
            "   MOV     r11, r7\n"
    
            // 4. Get the low regisers via LDM, which requires subtracting 32 bytes
            //    to get to the front of that block.
            "   SUB     %0, %0, #32\n"
            "   LDMIA   %0!, {r4-r7}\n"
    
            // 5. Add the DTSP by 16, to put it where it must be
            "   ADD     %0, %0, #16\n"
            
            // 6. Set PSP to DTSP, thus activating it
            "   MSR     psp, %0\n"
            );
        } break;
        
        
        // Other calls are process notifications
       default: //sys_notify();
                break;  //goto SVC_Handler_eventmgr;
    }

#endif
}



void PendSV_Handler(void) {
/// PendSV ISR is issued if and only if some task or event is asynchronously
/// pre-empting the kernel.  It is invoked __only__ through platform_ot_preempt().
/// @note SV is for "Supervisor," not "Sport Veloce"

    // There is an erratum that PendSV bit is not adequately cleared in HW
    __CLR_PENDSV();

    // Disable interrupts during scheduler runtime
    __SEND_SVC(0);
}


///@note these need to be implemented flatly into SVC

#ifndef EXTF_platform_save_context
OT_INLINE void platform_save_context(void) {
    __SEND_SVC(2);
}
#endif

#ifndef EXTF_platform_load_context
OT_INLINE void platform_load_context(void) {
    __SEND_SVC(3);
}
#endif

#ifndef EXTF_platform_switch_context
OT_INLINE void platform_switch_context(void) {
    __SEND_SVC(2);
}
#endif



#ifndef EXTF_platform_drop_context
void platform_drop_context(ot_uint i) {
/// This function should only be called from a kernel supervisor call (SVC).
/// In normal sytems it is used via sys_kill().  In applications using threads,
/// sys_kill() (or code by-way-of sys_kill()) is responsible for calling the
/// task exit function and emptying the thread stack.
    __SEND_SVC(1);
    
    /*
#   if (OT_PARAM_SYSTHREADS != 0)
        // Here would go some context clearing jazz
#   endif

    // Safety check!
    if (platform_ext.task_exit != NULL) {
        // Initial Task LR was placed on P-stack by NVIC HW when this interrupt
        // occurred.  By design it has a 6-word offset from PSP.  Overwrite it
        // with the saved address that sends code to RETURN_FROM_TASK.
        register ot_u32 task_lr;
        asm volatile ("MRS  %0, PSP" : "=r"(task_lr) );
        ((ot_u32*)task_lr)[6] = (ot_u32)platform_ext.task_exit;
    }
    */
}
#endif


#ifndef EXTF_platform_ot_preempt
void platform_ot_preempt() {
/// Run the kernel scheduler by using system call.  If running a kernel task,
/// do not run the scheduler (we check this by inspecting the KTIM interrupt
/// enabler bit), as the ktask will run to completion and do the call in its
/// own context (platform_ot_run()).

    if (platform_ext.task_exit == NULL)
        __SET_PENDSV();
}
#endif



#ifndef EXTF_platform_pause
//void platform_ot_pause() {
//    platform_ot_preempt();
//    platform_flush_systim();
//}
#endif



#ifndef EXTF_platform_ot_run
OT_INLINE void platform_ot_run() {
/// This function must be run in a while(1) loop from main.  It is the context
/// used by Kernel Tasks, which are co-operatively multitasked.

    /// 1. Run the Scheduler.  The scheduler will issue a PendSV if there is a
    /// threaded task, in which case the P-stack will get changed to that
    /// thread, and the code after this call will not run until all threads are
    /// dormant.
    __SEND_SVC(0);

    /// 2. When the PC is here, it means that a kernel task has been scheduled
    ///    or that no task is scheduled.  If no task is scheduled, then it is
    ///    time to go to sleep.  sys_powerdown() MUST re-enable interrupts
    ///    immediately before issuing WFI instruction.
    while (systim.flags & GPTIM_FLAG_SLEEP) {
        platform_disable_interrupts();
        systim_enable();
        sys_powerdown();
        
        // At this point, system has just woke-up from sleep.  
        // The wakeup ISR must clear GPTIM_FLAG_SLEEP
    }

    /// 3. Stop the backup timer, which is used as a kernel watchdog.
    systim_stop_ticker();
    
    /// 4. Save the current P-stack pointer (PSP), and push the return address
    ///    onto this position.  If the task is killed during its runtime, this
    ///    data will be used to reset the P-stack and PC.
    ///@note this code only works with GCC-based compilers.  The && operator
    /// ahead of the label is a label-reference, and it is a GCC feature.
    {   register ot_u32 return_from_task;
        platform_ext.task_exit  = (void*)__get_PSP();
        return_from_task        = (ot_u32)&&RETURN_FROM_TASK;
        asm volatile ("PUSH {%0}" : : "r"(return_from_task) );
    }

    /// 5. Run the Tasking Engine.  It will call the ktask or switch to the
    /// thread, as needed based on what is scheduled.
    sys_run_task();

    /// 6. In any condition, retract the stack to a known, stable condition.
    /// If the task/thread exited cleanly, this changes nothing.  If killed,
    /// this will flush the stack.
    RETURN_FROM_TASK:
    __set_PSP( (ot_u32)platform_ext.task_exit );
    platform_ext.task_exit = NULL;

}
#endif


