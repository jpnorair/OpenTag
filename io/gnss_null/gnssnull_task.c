/* Copyright 2017 Haystack Technologies, Inc.
  * Proprietary and Confidential, all rights reserved
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
