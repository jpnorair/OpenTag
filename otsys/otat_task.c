/* Copyright 2016 JP Norair
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
  * @file       /otsys/otat_task.c
  * @author     JP Norair
  * @version    R102
  * @date       20 Oct 2016
  * @brief      Exotask Implementing the OTAt Feature
  * @ingroup    System
  *
  * OTAt is sort of similar to UNIX "at," except that it is implemented as a
  * sysqueue.  OTAt runs in co-operative tasking mode, and it is tickless. It
  * may be used to schedule all kinds of infrequent features. Often it is used 
  * for the filesystem, long-duty network stack sync, etc.
  *
  ******************************************************************************
  */


#include <otsys/otat.h>
#include <otsys/sysqueue.h>
//#include <otlib/memcpy.h>
//#include <otlib/rand.h>

// For testing purposes, even though this should be included in the platform
#if (defined(__STDC__) || defined (__POSIX__))
#   include <stdio.h>
#endif




#if defined(OT_PARAM_CRONSIZE)
#   if (OT_PARAM_CRONSIZE < 4)
#       define _DEFSIZE     4
#   else
#       define _DEFSIZE     OT_PARAM_CRONSIZE
#   endif
#endif

#if (OS_FEATURE(MALLOC))
#   include <otlib/otmalloc.h>

    typedef struct otat_struct {
        ot_s32      delta0;
        ot_sq       sq;
        ot_sqnode*  heap;
    } otat_type;

#   define _CRONSIZE(SIZE)          (((SIZE) > _DEFSIZE) ? (SIZE) : _DEFSIZE) 
#   define _ALLOCATE(SIZE)          ot_malloc((SIZE) * sizeof(ot_sqnode))
#   define _EXITCHECK(PTR, CODE)    if (PTR == NULL) return CODE
#   define _FREE()                  ot_free(otat.heap)

#else
    typedef struct otat_struct {
        ot_s32      delta0;
        ot_sq       sq;
        ot_sqnode   heap[_DEFSIZE];
    } otat_type;

#   define _CRONSIZE(SIZE)          _DEFSIZE 
#   define _ALLOCATE(SIZE)          otat_heap
#   define _EXITCHECK(PTR, CODE);
#   define _FREE();

#endif



static otat_type otat;




/** otat API functions <BR>
  * =========================================================================== <BR>
  * 
  */

OT_WEAK void otat_kill() {
    sq_clear(otat.sq);
    _FREE();
}

OT_WEAK ot_int otat_init(ot_uint size) {
    ot_sqnode* heap;
    
    size = _CRONSIZE(size);
    heap = _ALLOCATE(size);
    _EXITCHECK(heap, -1);
    
    sq_init(&otat.sq, otat.heap, size);
    otat.clock = 0;
    return size;
}








/** Delta Computation subroutines <BR>
  * =========================================================================== <BR>
  * Timing delta is for managing multitasking delays.  Without this, if there
  * is a delay in one task all future tasks get delayed.  With it, momentarily
  * delays will not propagate.  Another option is to continually clock all the
  * elements of the otat queue in the kernel scheduler, but that is much more
  * expensive than managing a delta.
  *
  * sub_compute_delta0() : sets insertion delta (before otat task runs)
  * sub_compute_delta1() : sets exit delta (after otat task completes)
  */

void sub_compute_delta0(ot_task task, ot_sqnode* node) {
    ot_s32 t;
    
    // Necessary to disable interrupts during clocking phase.
    platform_disable_interrupts();
    
    // delta0 is the insertion delta, always non-negative
    // task->nextevent will be always <= 0 during this call
    // node counter may be positive or negative
    //  - if negative, it indicates we are behind schedule! So correct that.
    t = (ot_s32)((ot_sqnode_cnt32*)node)->counter;
    if (t < 0) {
        t = -t;   
    }
    otat.delta0 = t - task->nextevent;
    
    // Reset nextevent to clock runtime of the otat task
    task->nextevent = 0;
    
    // Clocking done, re-enable interrupts
    platform_enable_interrupts();
}


void sub_compute_delta1(ot_task task, ot_sqnode* node) {
    ot_s32 delta1;
    
    // Necessary to disable interrupts during clocking phase.
    platform_disable_interrupts();
    
    // here, task->nextevent is <= 0, and represents tasking overshoot
    // systim_get() returns >= 0, and represents kernel time
    // delta0 is >= 0, and represents the clock delta ahead of last task
    delta1 += otat.delta0 - task->nextevent + systim_get();
    
    // Set nextevent accordingly
    task->nextevent = delta1;
    
    // Clocking done, re-enable interrupts
    platform_enable_interrupts();
}




/** otat task <BR>
  * =========================================================================== <BR>
  * Otat uses a unique task clocker, which can be used as an example on how 
  * to clock task queues.
  */

void otat_systask(ot_task task) {
    switch (task->event) {
        
        // OTat task destructor is same as init
        case 0: otat_kill();
                break;

        // OTat task exit clocker
        // - Always runs closely after state 2
        // - Uses a trick with task->nextevent to clock elapsed time from
        //   executing in state 2.
        case 1: {
            ot_sqnode* node;
            
            // Get the node, exit if somehow the queue has been deleted
            node = sq_top(&otat.sq);
            if (node == NULL) {
                task->event = 0;
                break;   
            }
            
            // Compute the exit delta and setup otat for next task
            sub_compute_delta1(task, node);
            task->event = 2;
        } break;

        // OTat run the task
        // If the queue is not empty after the task runs, re-schedule with kernel
        case 2: {
            ot_sqnode* node;
            
            // Get the node, exit if somehow the queue has been deleted
            node = sq_pop(&otat.sq); 
            if (node == NULL) {
                task->event = 0;
                break;   
            }
            
            // Compute the insertion delta, then run the otat task from the node
            sub_compute_delta0(task, node);
            ((ot_atstub)node->handle)();

            // If queue is not empty, the kernel will loop and we can clock-out
            // the next queue item in state 1.
            task->event = (ot_u8)sq_notempty(&otat.sq);
        } break;
    }
}




