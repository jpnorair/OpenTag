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
  * @file       /hbsys/ubxm8_task.c
  * @author     JP Norair
  * @version    R101
  * @date       14 Apr 2017
  * @brief      UBX Protocol manager for GNSS positioning
  * @ingroup    UBX_GNSS
  *
  *
  ******************************************************************************
  */


#include <otstd.h>
#include <board.h>

#if (OT_FEATURE(GNSS) && BOARD_FEATURE(GNSSNULL))

#include <otsys.h>
#include <otlib.h>


/** External API functions
  * ========================================================================
  */
void gnssnull_config(ot_sig2 callback, ot_u16 onoff_interval) {
}

void gnssnull_connect(void) {
}

void gnssnull_disconnect(void) {
}

void gnssnull_agps_stamp(void) {
}


ot_int gnssnull_latlon_sprint(ot_u8* dst, ot_s32 latlon) {
    return 0;
}


ot_int gnssnull_distance_sprint(ot_u8* dst, ot_s32 distance) {
    return 0;
}





/** Main Task Function
  * ========================================================================
  */

void gnssnull_systask(ot_task task) {
/// It is optional to use a loop in a task function, but for this task it helps
/// reduce some load on the scheduler.
    ot_u32  nextevent = (ot_u32)-1;

    do {
        switch (task->event) {
        // Task destructor: close iap & iapdrv
        ///@todo update this for modern form with rebooter
        case 0: sys_taskinit_macro(task, otutils_null(), otutils_null());
                return;
         
        case 255: task->cursor = 3;
                  return;
                  
        default: {
            task->event = 1;
            nextevent   = (ot_u32)-1;
            } break;
        }
        
    } while ((nextevent <= TI2CLK(0)) && (task->event != 0));


    /// The task is done for now.  Instruct the Scheduler when to reinvoke this task.
    sys_task_setnext(task, nextevent);
}





#endif
