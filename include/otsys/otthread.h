/*  Copyright 2010-2014, JP Norair
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
  * @file       /include/otsys/otthread.h
  * @author     JP Norair
  * @version    R101
  * @date       27 Aug 2014
  * @brief      OpenTag Threads
  * @ingroup    System
  * 
  * OpenTag supports a threading system with basic features like:
  * <LI> Mutexes, both explicit and integrable into other data types </LI>
  * <LI> "Cond Signals" with maskable signaling (e.g. multicasting) </LI>
  * <LI> 
  ******************************************************************************
  */


#ifndef __OTSYS_OTTHREAD_H
#define __OTSYS_OTTHREAD_H

#include <otstd.h>
#include <otsys/syskern.h>


/** @brief ot_tid is the "Thread ID."  OpenTag implements this merely as an
  *        integer.  The maximum amount of threads is usually 16.
  */
typedef ot_uint ot_tid;


/** @brief ot_tmask is a Thread Mask.  It is implemented here merely as a bit
  *        mask, which is trivial since the maximum number of threads is 16.
  */
typedef ot_uint ot_tmask;


/** @brief ot_thread is a "Thread Handle" that can work directly with masking
  *        and signaling operations.  It is actually just a single-bit bitmask.
  */
typedef ot_uint ot_thread;




ot_thread otthread_self(void);

ot_tid otthread_self_tid(void);




ot_tid otthread_kernel_tid(void);

ot_thread otthread_kernel(void);


void otthread_set_tmask( ot_tmask tmask, ot_thandle thandle );

void otthread_clear_tmask( ot_tmask tmask, ot_thandle thandle );



void otthread_wait(void);



void otthread_release( ot_thandle thandle );

void otthread_release_any( ot_tmask tmask );




ot_int otthread_mutex_lock( ot_mutex* mutex );

ot_int otthread_mutex_trylock( ot_mutex* mutex );

ot_int otthread_mutex_unlock( ot_mutex* mutex );



#endif





