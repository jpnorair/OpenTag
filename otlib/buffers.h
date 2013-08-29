/* Copyright 2010-2011 JP Norair
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
  * @file       /OTlib/buffers.h
  * @author     JP Norair
  * @version    V1.0
  * @date       1 June 2011
  * @brief      An inline, volatile set of stream buffers used with Queues
  * @defgroup   Buffers (Buffers Module)
  * @ingroup    Buffers
  *
  ******************************************************************************
  */

#ifndef __BUFFERS_H
#define __BUFFERS_H

#include "OT_types.h"
#include "OT_config.h"
#include "queue.h"


/** Buffer Allocation definitions
  * These may also be defined in OT_config.h unless otherwise noted.  The
  * definitions in OT_config.h take priority over these.
  */

#define BUF0_ALLOC  256
#define BUF1_ALLOC  256
#define BUF2_ALLOC  (OT_FEATURE(BUFFER_SIZE) - 512)


/// Buffer Partitions
/// Number of partitions allowed is the total allocated size divided by 256.
/// Partitions, hence, are always 256 bytes.
#define BUF_PARTITIONS      (OT_FEATURE(BUFFER_SIZE)/256)
#define BUF_PARTITION(VAL)  (otbuf+(256*VAL))


// Legacy defines for certain hacks that may still be in the codebase. 
// Usage of these is deprecated
#define txq_data    (otbuf)
#define rxq_data    (otbuf+BUF0_ALLOC)


/// Common start points for queues in the buffer
#define ot_buf0     (otbuf)
#define ot_buf1     (otbuf+BUF0_ALLOC)
#define ot_buf2     (otbuf+BUF0_ALLOC+BUF1_ALLOC)


/// Main Buffer (encapsulates buffers for all supported Queues)
extern ot_u8 otbuf[OT_FEATURE(BUFFER_SIZE)];



#if (OT_FEATURE(SERVER) == ENABLED)
    /// Required Queues (on server side): rxq and txq are used for DASH7 I/O
    extern Queue rxq;
    extern Queue txq;
#endif

#if (   (OT_FEATURE(NDEF)  == ENABLED) || \
        (OT_FEATURE(ALP)   == ENABLED) || \
        (OT_FEATURE(MPIPE) == ENABLED) )
    /// Directive Queues: optional, used for ALP processing and usually also 
    /// NDEF record usage (if enabled)
    extern Queue otmpin;
    extern Queue otmpout;
#endif




/** @brief Initializes the System Queues to defaults
  * @param none
  * @retval none
  * @ingroup Buffers
  *
  * The System Queues (rxq, txq, otmpin, otmpout) can be manipulated to point
  * to different data elements during the course of their usage.  This function
  * will reset them to their defaults.  Use it whenever you want to reset the
  * queues or during boot-up.
  */
void buffers_init();




#endif
