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





