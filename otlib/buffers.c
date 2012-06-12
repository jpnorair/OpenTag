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
  * @file       /OTlib/buffers.c
  * @author     JP Norair
  * @version    V1.0
  * @date       1 June 2011
  * @brief      An inline, volatile set of stream buffers used with Queues
  * @ingroup    Buffers
  *
  ******************************************************************************
  */

#include "buffers.h"

ot_u8 otbuf[OT_FEATURE(BUFFER_SIZE)];

#if (OT_FEATURE(SERVER) == ENABLED)
    Queue rxq;
    Queue txq;
#endif

#if (   (OT_FEATURE(NDEF)  == ENABLED) || \
        (OT_FEATURE(ALP)   == ENABLED) || \
        (OT_FEATURE(MPIPE) == ENABLED) )
    Queue dir_in;
    Queue dir_out;
#endif



void buffers_init() {
    ot_int max;

#   if (OT_FEATURE(SERVER) == ENABLED)
        /// TX/RX queues
        max = M2_PARAM_MAXFRAME + (M2_PARAM_MAXFRAME & 1);  //keep even
        q_init(&rxq, otbuf, max);
        q_init(&txq, otbuf+max, max);
#   else
        max = 0;
#   endif
    
#if (   (OT_FEATURE(NDEF)  == ENABLED) || \
        (OT_FEATURE(ALP)   == ENABLED) || \
        (OT_FEATURE(MPIPE) == ENABLED) )
        /// Console queues can use the same space (half duplex).  For heavy
        /// clients with memory to spare, feel free to make full duplex.
        max <<= 1;  // (max *= 2)
        q_init(&dir_in, otbuf+max, (OT_FEATURE(BUFFER_SIZE)-max) );    
        q_init(&dir_out, otbuf+max, (OT_FEATURE(BUFFER_SIZE)-max) );    
#   endif
}


/// Experimental
void buffers_swap(Queue* q1, Queue* q2) {
    Queue scratch;
    
    q_copy(&scratch, q1);
    q_copy(q1, q2);
    q_copy(q2, &scratch);
}




