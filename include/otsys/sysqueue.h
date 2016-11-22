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
  * @file       /include/otsys/sysqueue.h
  * @author     JP Norair
  * @version    R103
  * @date       20 Oct 2016
  * @brief      System Queue (Queue for task calls)
  * @ingroup    System
  *
  * OpenTag Tasks can utilize a System Queue -- although it is optional.  Many
  * low-level tasks in OpenTag implement their own, unique scheduling procedures
  * A task utilizing a system queue is like a traditional OS scheduler, and it 
  * can perform whatever actions occur in its sysqueue however it wants.
  *
  * OpenTag implements native threading and/or guest operating systems through
  * this sysqueue.  OpenTag can have more than one sysqueue, effectively 
  * allowing it to implement multiple guest operating systems on top of itself,
  * thus allowing fast integration of other stacks, applications, toolchains,
  * etc, that are built onto other OSes.
  * 
  ******************************************************************************
  */


#ifndef __SYSQUEUE_h
#define __SYSQUEUE_h

#include <otsys/types.h>
#include <otsys/config.h>



/** @typedef ot_sqnode
  * @brief  OpenTag System Queue Node (ot_sqnode)
  *
  * handle      (void*) Arbitrary pointer to some data that the queue manager
  *             and nodal process should know how to utilize.
  * 
  * counter     (ot_u16) A number of time units indicating queue schedule.
  *             Often, these are ticks (ti), but they could be arbitrary units.
  *             When counter == 0, the queue manager should activate the node.
  *             The manager can use sq_clock() (or other sysqueue module 
  *             functions) to manage the counter.
  *
  * ext         (ot_u16) Arbitrary data.  In many uses, it behaves as lower
  *             16 bits of a 32 bit counter.  In other cases, as flags.
  */
typedef struct ot_sqnode {
    void*   handle;
    ot_u16  counter;
    ot_u16  ext;
} ot_sqnode;



/** @typedef ot_sqcall
  * @brief Applet call that is associated with a Sysqueue node
  *
  * Call associated with a queue node.  The queue manager should be able to 
  * call this to enact some functionality.  For example, if the queue is 
  * holding *thread events,* this will invoke the thread handling function.
  */
typedef void (*ot_sqcall)(ot_sqnode*);


/** @typedef ot_sqcmp
  * @brief Comparison function for sorting nodes
  *
  * Should output -1, 0, 1 if the first argument is less than, equal to, or 
  * greater than the second argument. The client should implement this function
  * to meet the needs of how it wants to sort nodes in the queue.
  */
typedef ot_int (*ot_sqcmp)(ot_sqnode*, ot_sqnode*);


/** @typedef ot_sqcond
  * @brief Condition function for evaluating nodes
  *
  * Should output True or False if the parameters from the provided node
  * indicate the some user-specified condition
  */
typedef ot_bool (*ot_sqcond)(ot_sqnode*);



/** @typedef ot_sq
  * @brief OpenTag System Queue structure
  *
  * The manager of the queue (often an Exotask managed by the kernel) will need
  * to declare an ot_sq for its usage, and also declare an array of ot_sqnodes.
  * These get attached together in sq_init().
  */
typedef struct {
    ot_sqnode*  top;
    ot_sqnode*  heap;
    ot_uint     length;
    ot_uint     size;
} ot_sq;




void sq_call_null(ot_sqnode* a);



/** sq_init() <BR>
  * ======================================================================= <BR>
  */

void sq_init(ot_sq* sq, ot_sqnode* array, ot_uint size);




/** sq_activate...() functions <BR>
  * ======================================================================= <BR>
  * "Activate" functions find the counter value on the top node, return it,
  * and they set the stored couter value to zero.
  */

ot_uint sq_activate(ot_sq* sq);

ot_u32 sq_activate_cnt32(ot_sq* sq);



/** sq_clock...() functions <BR>
  * ======================================================================= <BR>
  * "Clock" functions subtract an amount of ticks from each node counter.
  * Lowest value is 0.  They return the counter value of the top node.
  */

ot_uint sq_clock(ot_sq* sq, ot_u16 ticks);

ot_u32 sq_clock_cnt32(ot_sq* sq, ot_u32 ticks);





/** sq_new...() functions <BR>
  * ======================================================================= <BR>
  * sq_new...() variants use a binary insertion sort to place a new node in
  * the right place in the queue.  The client gets a comparison function
  * callback in order to determine how the queue should be sorted.
  */
ot_sqnode* sq_new(ot_sq* sq, ot_sqcmp cmpfn, ot_sqnode* node);

ot_sqnode* sq_new_fromcnt32(ot_sq* sq, ot_sqcmp cmpfn, void* handle, ot_u32 wait32);

ot_sqnode* sq_new_fromargs(ot_sq* sq, ot_sqcmp cmpfn, void* handle, ot_u16 wait, ot_u16 ext);








/** sq_extend...() functions <BR>
  * ======================================================================= <BR>
  * sq_extend...() variants use a linear search starting from the front of
  * the queue.  If you have a bunch of sequenced nodes that must stay in
  * sequence you use extend to plop a new node after them.
  */

ot_sqnode* sq_extend(ot_sq* sq, ot_sqcond condfn, ot_sqnode* node);

ot_sqnode* sq_extend_fromcnt32(ot_sq* sq, ot_sqcond condfn, void* handle, ot_u32 wait32);

ot_sqnode* sq_extend_fromargs(ot_sq* sq, ot_sqcond condfn, void* handle, ot_u16 wait, ot_u16 ext);




/** sq data API functions <BR>
  * ======================================================================= <BR>
  */

void sq_pop(ot_sq* sq);

void sq_flush(ot_sq* sq, ot_sqcond condfn);

ot_sqnode* sq_top(ot_sq* sq);

ot_sqnode* sq_follower(ot_sq* sq);

ot_int sq_numfree(ot_sq* sq);

ot_bool sq_notempty(ot_sq* sq);




#if (defined(__STDC__) || defined (__POSIX__))

/** @brief  Test function to print the session stack to stdout (POSIX/STD-C only)
  * @param  none
  * @retval None
  * @ingroup Session
  */
void session_print();
#endif

#endif
