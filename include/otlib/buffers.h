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
  * @file       /otlib/buffers.h
  * @author     JP Norair
  * @version    R100
  * @date       8 Oct 2013
  * @brief      An inline, volatile set of stream buffers used with OT ot_queues
  * @defgroup   Buffers (Buffers Module)
  * @ingroup    Buffers
  *
  ******************************************************************************
  */

#ifndef __BUFFERS_H
#define __BUFFERS_H

#include <otstd.h>
#include <otlib/queue.h>


/** Buffer Allocation definitions
  * These may also be defined in OT_config.h unless otherwise noted.  The
  * definitions in OT_config.h take priority over these.
  */

#define BUF0_ALLOC  256
#define BUF1_ALLOC  256
#define BUF2_ALLOC  (OT_PARAM(BUFFER_SIZE) - 512)


/// Buffer Partitions
/// Number of partitions allowed is the total allocated size divided by 256.
/// Partitions, hence, are always 256 bytes.
#define BUF_PARTITIONS      (OT_PARAM(BUFFER_SIZE)/256)
#define BUF_PARTITION(VAL)  (otbuf+(256*VAL))


// Legacy defines for certain hacks that may still be in the codebase.
// Usage of these is deprecated
#define txq_data    (otbuf)
#define rxq_data    (otbuf+BUF0_ALLOC)


/// Common start points for queues in the buffer
#define ot_buf0     (otbuf)
#define ot_buf1     (otbuf+BUF0_ALLOC)
#define ot_buf2     (otbuf+BUF0_ALLOC+BUF1_ALLOC)


/// Main Buffer (encapsulates buffers for all supported ot_queues)
extern ot_u8 otbuf[OT_PARAM(BUFFER_SIZE)];



#if (OT_FEATURE(SERVER) && OT_FEATURE(M2))
    /// Required ot_queues (on server side): rxq and txq are used for DASH7 I/O
    extern ot_queue rxq;
    extern ot_queue txq;
#endif

#if (OT_FEATURE(NDEF) || OT_FEATURE(ALP) || OT_FEATURE(MPIPE))
    /// Analagous to stdin/stdout.  Frequently used ALP application queues
    extern ot_queue otmpin;
    extern ot_queue otmpout;


#endif




/** @brief Initializes the System ot_queues to defaults
  * @param none
  * @retval none
  * @ingroup Buffers
  *
  * The System ot_queues (rxq, txq, otmpin, otmpout) can be manipulated to point
  * to different data elements during the course of their usage.  This function
  * will reset them to their defaults.  Use it whenever you want to reset the
  * queues or during boot-up.
  */
void buffers_init();




#endif
